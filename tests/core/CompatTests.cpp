// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/CompatTests.cpp
/// @brief  Unit tests for the feature-test wrappers and fallbacks in core/Compat.h (T-01, L-04).

#include "core/Compat.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace compat = app::core::compat;

namespace
{

template <typename View>
std::vector<std::pair<std::ptrdiff_t, int>> collectIndexed( View&& view )
{
    std::vector<std::pair<std::ptrdiff_t, int>> result;

    for( auto&& [index, value] : view )
    {
        result.emplace_back( static_cast<std::ptrdiff_t>( index ), value );
    }

    return result;
}

template <typename View>
std::vector<std::pair<int, std::string>> collectPairs( View&& view )
{
    std::vector<std::pair<int, std::string>> result;

    for( auto&& [number, text] : view )
    {
        result.emplace_back( number, text );
    }

    return result;
}

} // namespace

TEST( Compat, FlagsMatchFeatureTestMacros_WhenCompiled )
{
    EXPECT_EQ( compat::kHasRangesZip, APP_HAS_RANGES_ZIP != 0 );
    EXPECT_EQ( compat::kHasRangesEnumerate, APP_HAS_RANGES_ENUMERATE != 0 );
    EXPECT_EQ( compat::kHasMdspan, APP_HAS_MDSPAN != 0 );
    EXPECT_EQ( compat::kHasFlatMap, APP_HAS_FLAT_MAP != 0 );
    EXPECT_EQ( compat::kHasFlatSet, APP_HAS_FLAT_SET != 0 );

#if defined( __cpp_lib_ranges_enumerate )
    EXPECT_EQ( compat::kHasRangesEnumerate, __cpp_lib_ranges_enumerate >= 202302L );
#else
    EXPECT_FALSE( compat::kHasRangesEnumerate );
#endif
}

TEST( Compat, EnumerateYieldsIndexAndValue_WhenIteratingVector )
{
    const std::vector<int>                            values{ 10, 20, 30 };
    const std::vector<std::pair<std::ptrdiff_t, int>> expected{ { 0, 10 }, { 1, 20 }, { 2, 30 } };

    EXPECT_EQ( collectIndexed( compat::enumerate( values ) ), expected );
    EXPECT_EQ( collectIndexed( compat::fallback::enumerate( values ) ), expected );
}

TEST( Compat, EnumerateWritesThroughReferences_WhenRangeIsMutable )
{
    std::vector<int> standard{ 1, 1, 1 };
    std::vector<int> fallback{ 1, 1, 1 };

    for( auto&& [index, value] : compat::enumerate( standard ) )
    {
        value += static_cast<int>( index );
    }

    for( auto&& [index, value] : compat::fallback::enumerate( fallback ) )
    {
        value += static_cast<int>( index );
    }

    EXPECT_EQ( standard, ( std::vector<int>{ 1, 2, 3 } ) );
    EXPECT_EQ( fallback, standard );
}

TEST( Compat, EnumerateIsEmpty_WhenRangeIsEmpty )
{
    const std::vector<int> empty;

    EXPECT_TRUE( collectIndexed( compat::enumerate( empty ) ).empty() );
    EXPECT_TRUE( collectIndexed( compat::fallback::enumerate( empty ) ).empty() );
}

TEST( Compat, ZipStopsAtShorterRange_WhenLengthsDiffer )
{
    const std::vector<int>                         numbers{ 1, 2, 3 };
    const std::vector<std::string>                 names{ "one", "two" };
    const std::vector<std::pair<int, std::string>> expected{ { 1, "one" }, { 2, "two" } };

    EXPECT_EQ( collectPairs( compat::zip( numbers, names ) ), expected );
    EXPECT_EQ( collectPairs( compat::fallback::zip( numbers, names ) ), expected );
}

TEST( Compat, ZipWritesThroughReferences_WhenRangesAreMutable )
{
    std::vector<int>       left{ 1, 2, 3 };
    const std::vector<int> right{ 10, 20, 30 };
    std::vector<int>       leftFallback = left;

    for( auto&& [a, b] : compat::zip( left, right ) )
    {
        a += b;
    }

    for( auto&& [a, b] : compat::fallback::zip( leftFallback, right ) )
    {
        a += b;
    }

    EXPECT_EQ( left, ( std::vector<int>{ 11, 22, 33 } ) );
    EXPECT_EQ( leftFallback, left );
}

TEST( Compat, ZipTransformAppliesFunctionPairwise_WhenIterated )
{
    const std::vector<int> a{ 1, 2, 3, 4 };
    const std::vector<int> b{ 5, 6, 7 };
    const auto             multiply = []( int x, int y )
    {
        return x * y;
    };

    std::vector<int> standard;
    std::vector<int> fallback;

    for( int product : compat::zipTransform( multiply, a, b ) )
    {
        standard.push_back( product );
    }

    for( int product : compat::fallback::zipTransform( multiply, a, b ) )
    {
        fallback.push_back( product );
    }

    EXPECT_EQ( standard, ( std::vector<int>{ 5, 12, 21 } ) );
    EXPECT_EQ( fallback, standard );
}

TEST( Compat, FlatMapKeepsKeysSorted_WhenInsertedOutOfOrder )
{
    compat::FlatMap<int, std::string>           map;
    compat::fallback::FlatMap<int, std::string> fallback;

    for( const auto& [key, value] : std::vector<std::pair<int, std::string>>{ { 3, "c" }, { 1, "a" }, { 2, "b" } } )
    {
        map.emplace( key, value );
        fallback.emplace( key, value );
    }

    map[4]      = "d";
    fallback[4] = "d";

    std::string mapOrder;
    std::string fallbackOrder;

    for( const auto& [key, value] : map )
    {
        mapOrder += value;
    }

    for( const auto& [key, value] : fallback )
    {
        fallbackOrder += value;
    }

    EXPECT_EQ( mapOrder, "abcd" );
    EXPECT_EQ( fallbackOrder, mapOrder );
    EXPECT_EQ( map.at( 2 ), "b" );
    EXPECT_TRUE( fallback.contains( 3 ) );
    EXPECT_EQ( map.erase( 1 ), 1U );
    EXPECT_EQ( fallback.erase( 1 ), 1U );
    EXPECT_EQ( map.size(), fallback.size() );
}

TEST( Compat, FlatSetRemovesDuplicates_WhenInserted )
{
    compat::FlatSet<int>           set;
    compat::fallback::FlatSet<int> fallback;

    for( int value : { 5, 1, 5, 3, 1 } )
    {
        set.insert( value );
        fallback.insert( value );
    }

    EXPECT_EQ( std::vector<int>( set.begin(), set.end() ), ( std::vector<int>{ 1, 3, 5 } ) );
    EXPECT_EQ( std::vector<int>( fallback.begin(), fallback.end() ), ( std::vector<int>{ 1, 3, 5 } ) );
}
