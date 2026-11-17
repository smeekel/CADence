// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/ObjectIdTests.cpp
/// @brief  Unit tests for app::core::ObjectId and ObjectIdGenerator (T-01, A-04).

#include "core/ObjectId.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

using app::core::ObjectId;
using app::core::ObjectIdGenerator;

namespace
{

// RFC 9562, appendix A.6: example UUIDv7 for 2022-02-22 19:22:22 UTC.
constexpr std::string_view kRfcExampleText = "017f22e2-79b0-7cc3-98c4-dc0c0c07398f";

constexpr ObjectId::Bytes kRfcExampleBytes =
    { 0x01, 0x7F, 0x22, 0xE2, 0x79, 0xB0, 0x7C, 0xC3, 0x98, 0xC4, 0xDC, 0x0C, 0x0C, 0x07, 0x39, 0x8F };

// Random source for tests that inject their own clock: a Weyl sequence, distinct on every call.
struct CountingRandom
{
    std::uint64_t state;

    std::uint64_t operator()()
    {
        state += 0x9E3779B97F4A7C15ULL;
        return state;
    }
};

} // namespace

TEST( ObjectId, IsNil_WhenDefaultConstructed )
{
    const ObjectId id;

    EXPECT_TRUE( id.isNil() );
    EXPECT_EQ( id.version(), 0 );
    EXPECT_EQ( id.toString(), "00000000-0000-0000-0000-000000000000" );
}

// Accept: A-04
TEST( ObjectId, HasVersion7AndRfcVariant_WhenGenerated )
{
    ObjectIdGenerator generator = ObjectIdGenerator::makeDeterministic( 7, 1'700'000'000'000 );

    for( int i = 0; i < 1000; ++i )
    {
        const ObjectId id = generator.next();

        EXPECT_EQ( id.version(), 7 );
        EXPECT_TRUE( id.hasRfcVariant() );
        EXPECT_FALSE( id.isNil() );
    }

    const ObjectId fromDefault = app::core::newObjectId();

    EXPECT_EQ( fromDefault.version(), 7 );
    EXPECT_TRUE( fromDefault.hasRfcVariant() );
}

TEST( ObjectId, StoresTimestampInFirst48Bits_WhenGenerated )
{
    ObjectIdGenerator generator = ObjectIdGenerator::makeDeterministic( 1, 0x0123'4567'89ABULL );
    const ObjectId    id        = generator.next();

    EXPECT_EQ( id.unixMilliseconds(), 0x0123'4567'89ABULL );
    EXPECT_EQ( id.bytes()[0], 0x01 );
    EXPECT_EQ( id.bytes()[5], 0xAB );
    EXPECT_EQ( id.toString().substr( 0, 13 ), "01234567-89ab" );
}

// Accept: A-04
TEST( ObjectId, GeneratesOneMillionUniqueIncreasingIds_WhenSeedIsFixed )
{
    ObjectIdGenerator generator = ObjectIdGenerator::makeDeterministic( 20261116, 1'763'251'200'000 );
    ObjectId          previous  = generator.next();

    for( int i = 1; i < 1'000'000; ++i )
    {
        const ObjectId id = generator.next();

        // Strictly increasing implies unique.
        ASSERT_LT( previous, id ) << "at index " << i;
        previous = id;
    }
}

TEST( ObjectId, ProducesSameSequence_WhenSeedAndClockMatch )
{
    ObjectIdGenerator first  = ObjectIdGenerator::makeDeterministic( 99, 1'000 );
    ObjectIdGenerator second = ObjectIdGenerator::makeDeterministic( 99, 1'000 );
    ObjectIdGenerator other  = ObjectIdGenerator::makeDeterministic( 100, 1'000 );

    bool differs = false;

    for( int i = 0; i < 100; ++i )
    {
        const ObjectId id = first.next();

        EXPECT_EQ( id, second.next() );
        differs = differs || id != other.next();
    }

    EXPECT_TRUE( differs );
}

TEST( ObjectId, OrdersByTimestamp_WhenClockAdvances )
{
    std::uint64_t     now = 5'000;
    ObjectIdGenerator generator(
        [&now]
        {
            return now;
        },
        CountingRandom{ 3 } );

    const ObjectId early = generator.next();

    now += 1;

    const ObjectId later = generator.next();

    EXPECT_LT( early, later );
    EXPECT_EQ( early.unixMilliseconds(), 5'000U );
    EXPECT_EQ( later.unixMilliseconds(), 5'001U );
}

TEST( ObjectId, StaysIncreasing_WhenClockGoesBackwards )
{
    std::uint64_t     now = 10'000;
    ObjectIdGenerator generator(
        [&now]
        {
            return now;
        },
        CountingRandom{ 4 } );

    const ObjectId before = generator.next();

    now = 9'000;

    const ObjectId after = generator.next();

    EXPECT_LT( before, after );
    EXPECT_EQ( after.unixMilliseconds(), 10'000U );
}

TEST( ObjectId, FormatsLowercaseHyphenated_WhenConvertedToString )
{
    const ObjectId id = ObjectId::fromBytes( kRfcExampleBytes );

    EXPECT_EQ( id.toString(), kRfcExampleText );
    EXPECT_EQ( id.version(), 7 );
    EXPECT_TRUE( id.hasRfcVariant() );
    EXPECT_EQ( id.unixMilliseconds(), 0x017F'22E2'79B0ULL );
}

TEST( ObjectId, KeepsRfc4122ByteOrder_WhenParsed )
{
    // The file-format magic (F-09) and its BSON byte sequence.
    const std::optional<ObjectId> magic = ObjectId::parse( "d9a3877a-3f27-4df8-9528-f8dfde59773d" );

    const ObjectId::Bytes expected =
        { 0xD9, 0xA3, 0x87, 0x7A, 0x3F, 0x27, 0x4D, 0xF8, 0x95, 0x28, 0xF8, 0xDF, 0xDE, 0x59, 0x77, 0x3D };

    EXPECT_EQ( magic, std::optional<ObjectId>( ObjectId::fromBytes( expected ) ) );
    EXPECT_EQ( magic.value_or( ObjectId{} ).version(), 4 );
}

TEST( ObjectId, RoundTripsThroughString_WhenParsed )
{
    ObjectIdGenerator generator = ObjectIdGenerator::makeDeterministic( 11, 1'234'567 );

    for( int i = 0; i < 1000; ++i )
    {
        const ObjectId id = generator.next();

        ASSERT_EQ( ObjectId::parse( id.toString() ), std::optional<ObjectId>( id ) );
    }
}

// ADR 0028: only the lower-case form is canonical, so a text UUID always round-trips byte-identically.
TEST( ObjectId, RejectsText_WhenHexDigitsAreUppercase )
{
    EXPECT_FALSE( ObjectId::parse( "017F22E2-79B0-7CC3-98C4-DC0C0C07398F" ).has_value() );
    EXPECT_FALSE( ObjectId::parse( "017f22e2-79b0-7cc3-98c4-dc0c0c07398F" ).has_value() );
    EXPECT_TRUE( ObjectId::parse( kRfcExampleText ).has_value() );
}

TEST( ObjectId, RejectsText_WhenNotInCanonicalForm )
{
    const std::string_view malformed[] = {
        "",
        "017f22e2-79b0-7cc3-98c4-dc0c0c07398",   // 35 characters
        "017f22e2-79b0-7cc3-98c4-dc0c0c07398f0", // 37 characters
        "017f22e279b07cc398c4dc0c0c07398f",      // no hyphens
        "{017f22e2-79b0-7cc3-98c4-dc0c0c0739}",  // braces
        "017f22e2-79b0-7cc3-98c4_dc0c0c07398f",  // wrong separator
        "017f22e-279b0-7cc3-98c4-dc0c0c07398f",  // hyphen in the wrong place
        "017f22g2-79b0-7cc3-98c4-dc0c0c07398f",  // not hexadecimal
        " 17f22e2-79b0-7cc3-98c4-dc0c0c07398f",  // whitespace
    };

    for( std::string_view text : malformed )
    {
        EXPECT_FALSE( ObjectId::parse( text ).has_value() ) << '"' << text << '"';
    }
}

TEST( ObjectId, HashesConsistentlyWithEquality_WhenUsedAsKey )
{
    ObjectIdGenerator               generator = ObjectIdGenerator::makeDeterministic( 5, 42 );
    std::vector<ObjectId>           ids;
    std::unordered_set<ObjectId>    set;
    std::unordered_set<std::size_t> hashes;

    ids.reserve( 100'000 );

    for( int i = 0; i < 100'000; ++i )
    {
        ids.push_back( generator.next() );
    }

    for( const ObjectId& id : ids )
    {
        set.insert( id );
        hashes.insert( std::hash<ObjectId>{}( id ) );
    }

    EXPECT_EQ( set.size(), ids.size() );
    EXPECT_EQ( hashes.size(), ids.size() );

    for( const ObjectId& id : ids )
    {
        const ObjectId copy = ObjectId::fromBytes( id.bytes() );

        ASSERT_TRUE( set.contains( copy ) );
        ASSERT_EQ( std::hash<ObjectId>{}( copy ), std::hash<ObjectId>{}( id ) );
    }
}
