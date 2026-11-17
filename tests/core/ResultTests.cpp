// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/ResultTests.cpp
/// @brief  Unit tests for app::Error and app::Result (T-01, ADR 0042).

#include "core/Result.h"

#include <gtest/gtest.h>

#include <set>
#include <string>
#include <string_view>

namespace
{

app::Result<int> parsePositive( int value )
{
    if( value <= 0 )
    {
        return app::fail( app::ErrorCode::InvalidArgument, "Value must be positive.", "value" );
    }

    return value;
}

app::Result<> requireEven( int value )
{
    if( value % 2 != 0 )
    {
        return app::fail( app::ErrorCode::Conflict, "Value must be even." );
    }

    return app::ok();
}

} // namespace

TEST( Error, NamesEveryCodeDistinctlyInKebabCase_WhenConverted )
{
    const app::ErrorCode       codes[] = { app::ErrorCode::InvalidArgument,
                                           app::ErrorCode::NotFound,
                                           app::ErrorCode::Conflict,
                                           app::ErrorCode::GeometryFailure,
                                           app::ErrorCode::FormatError,
                                           app::ErrorCode::IoError,
                                           app::ErrorCode::Unsupported,
                                           app::ErrorCode::Cancelled,
                                           app::ErrorCode::Internal };
    std::set<std::string_view> names;

    for( app::ErrorCode code : codes )
    {
        const std::string_view name = app::toString( code );

        EXPECT_FALSE( name.empty() );
        EXPECT_EQ( name.find_first_not_of( "abcdefghijklmnopqrstuvwxyz-" ), std::string_view::npos ) << name;
        names.insert( name );
    }

    EXPECT_EQ( names.size(), std::size( codes ) );
    EXPECT_EQ( app::toString( app::ErrorCode::InvalidArgument ), "invalid-argument" );
    EXPECT_EQ( app::toString( app::ErrorCode::IoError ), "io-error" );
}

TEST( Error, ReportsUnknown_WhenCodeIsOutOfRange )
{
    const auto outOfRange =
        static_cast<app::ErrorCode>( 200 ); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange): the case under test

    EXPECT_EQ( app::toString( outOfRange ), "unknown" );
}

TEST( Error, DescribeIncludesCodeMessageAndDetail_WhenDetailIsSet )
{
    const app::Error error{ app::ErrorCode::NotFound, "Parameter not found.", "Board.Width" };

    EXPECT_EQ( error.describe(), "not-found: Parameter not found. (Board.Width)" );
}

TEST( Error, DescribeOmitsParentheses_WhenDetailIsEmpty )
{
    const app::Error error{ app::ErrorCode::Cancelled, "Regeneration cancelled.", {} };

    EXPECT_EQ( error.describe(), "cancelled: Regeneration cancelled." );
}

TEST( Result, HoldsValue_WhenOperationSucceeds )
{
    const app::Result<int> result = parsePositive( 4 );

    ASSERT_TRUE( result.has_value() );
    EXPECT_EQ( *result, 4 );
}

TEST( Result, HoldsErrorFromFail_WhenOperationFails )
{
    const app::Result<int> result = parsePositive( -1 );

    ASSERT_FALSE( result.has_value() );
    EXPECT_EQ( result.error(), ( app::Error{ app::ErrorCode::InvalidArgument, "Value must be positive.", "value" } ) );
}

TEST( Result, VoidResultCarriesSuccessOrError_WhenReturned )
{
    EXPECT_TRUE( requireEven( 2 ).has_value() );
    EXPECT_TRUE( app::ok().has_value() );

    const app::Result<> odd = requireEven( 3 );

    ASSERT_FALSE( odd.has_value() );
    EXPECT_EQ( odd.error().code, app::ErrorCode::Conflict );
}

TEST( Result, ChainingStopsAtFirstError_WhenUsingAndThen )
{
    int calls = 0;

    const auto run = [&calls]( int input )
    {
        return parsePositive( input ).and_then(
            [&calls]( int value ) -> app::Result<int>
            {
                ++calls;
                return requireEven( value ).transform(
                    [value]()
                    {
                        return value * 10;
                    } );
            } );
    };

    const app::Result<int> good     = run( 4 );
    const app::Result<int> notEven  = run( 3 );
    const app::Result<int> negative = run( -2 );

    ASSERT_TRUE( good.has_value() );
    EXPECT_EQ( *good, 40 );
    EXPECT_EQ( notEven.error().code, app::ErrorCode::Conflict );
    EXPECT_EQ( negative.error().code, app::ErrorCode::InvalidArgument );
    EXPECT_EQ( calls, 2 );
}
