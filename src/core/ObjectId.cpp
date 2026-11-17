// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/ObjectId.cpp
/// @brief  UUID version 7 parsing, formatting and generation.

#include "core/ObjectId.h"

#include <chrono>

#include <random>
#include <utility>

namespace app::core
{

namespace
{

constexpr std::size_t   kTextLength    = 36;
constexpr std::uint64_t kTimestampMask = ( std::uint64_t{ 1 } << 48 ) - 1;
constexpr int           kCounterBits   = 42;
constexpr std::uint64_t kCounterMax    = ( std::uint64_t{ 1 } << kCounterBits ) - 1;

// The counter seed leaves the top bit clear so at least 2^41 IDs fit in one millisecond (RFC 9562 guard bit).
constexpr std::uint64_t kCounterSeedMask = ( std::uint64_t{ 1 } << ( kCounterBits - 1 ) ) - 1;

constexpr bool isHyphenPosition( std::size_t index ) noexcept
{
    return index == 8 || index == 13 || index == 18 || index == 23;
}

constexpr int hexValue( char c ) noexcept
{
    if( c >= '0' && c <= '9' )
    {
        return c - '0';
    }

    // Upper case is rejected: ADR 0028 allows only the lower-case form, so text and binary UUIDs round-trip exactly.
    if( c >= 'a' && c <= 'f' )
    {
        return c - 'a' + 10;
    }

    return -1;
}

std::mt19937_64 makeSeededEngine()
{
    std::random_device device;
    std::seed_seq      seed{ device(), device(), device(), device(), device(), device(), device(), device() };

    return std::mt19937_64( seed );
}

} // namespace

std::optional<ObjectId> ObjectId::parse( std::string_view text ) noexcept
{
    if( text.size() != kTextLength )
    {
        return std::nullopt;
    }

    Bytes       bytes{};
    std::size_t byteIndex = 0;
    int         high      = -1;

    for( std::size_t i = 0; i < kTextLength; ++i )
    {
        if( isHyphenPosition( i ) )
        {
            if( text[i] != '-' )
            {
                return std::nullopt;
            }

            continue;
        }

        const int value = hexValue( text[i] );

        if( value < 0 )
        {
            return std::nullopt;
        }

        if( high < 0 )
        {
            high = value;
        }
        else
        {
            bytes[byteIndex++] = static_cast<std::uint8_t>( ( high << 4 ) | value );
            high               = -1;
        }
    }

    return fromBytes( bytes );
}

std::string ObjectId::toString() const
{
    static constexpr char kDigits[] = "0123456789abcdef";

    std::string text;

    text.reserve( kTextLength );

    for( std::size_t i = 0; i < kSize; ++i )
    {
        if( i == 4 || i == 6 || i == 8 || i == 10 )
        {
            text += '-';
        }

        text += kDigits[m_bytes[i] >> 4];
        text += kDigits[m_bytes[i] & 0x0F];
    }

    return text;
}

ObjectIdGenerator::ObjectIdGenerator()
    : m_clock(
          []
          {
              const auto sinceEpoch = std::chrono::system_clock::now().time_since_epoch();
              const auto ms         = std::chrono::duration_cast<std::chrono::milliseconds>( sinceEpoch ).count();

              return ms > 0 ? static_cast<std::uint64_t>( ms ) : std::uint64_t{ 0 };
          } )
    , m_random(
          [engine = makeSeededEngine()]() mutable
          {
              return engine();
          } )
{
}

ObjectIdGenerator::ObjectIdGenerator( Clock clock, RandomSource random )
    : m_clock( std::move( clock ) )
    , m_random( std::move( random ) )
{
}

ObjectIdGenerator ObjectIdGenerator::makeDeterministic( std::uint64_t seed, std::uint64_t unixMs )
{
    return ObjectIdGenerator(
        [unixMs]
        {
            return unixMs;
        },
        [engine = std::mt19937_64( seed )]() mutable
        {
            return engine();
        } );
}

ObjectId ObjectIdGenerator::next()
{
    std::uint64_t timestamp = 0;
    std::uint64_t counter   = 0;
    std::uint64_t random    = 0;

    {
        const std::lock_guard lock( m_mutex );
        const std::uint64_t   now = m_clock() & kTimestampMask;

        if( !m_started || now > m_lastMs )
        {
            m_lastMs  = now;
            m_counter = m_random() & kCounterSeedMask;
            m_started = true;
        }
        else if( m_counter < kCounterMax )
        {
            // Same millisecond, or the clock went backwards: keep the last timestamp so IDs stay ordered.
            ++m_counter;
        }
        else
        {
            m_lastMs  = ( m_lastMs + 1 ) & kTimestampMask;
            m_counter = m_random() & kCounterSeedMask;
        }

        timestamp = m_lastMs;
        counter   = m_counter;
        random    = m_random();
    }

    const std::uint64_t randA = counter >> 30;
    const std::uint64_t randB = ( ( counter & 0x3FFFFFFF ) << 32 ) | ( random & 0xFFFFFFFF );

    ObjectId::Bytes bytes{};

    for( std::size_t i = 0; i < 6; ++i )
    {
        bytes[i] = static_cast<std::uint8_t>( timestamp >> ( 8 * ( 5 - i ) ) );
    }

    bytes[6] = static_cast<std::uint8_t>( 0x70 | ( randA >> 8 ) );
    bytes[7] = static_cast<std::uint8_t>( randA );
    bytes[8] = static_cast<std::uint8_t>( 0x80 | ( randB >> 56 ) );

    for( std::size_t i = 9; i < ObjectId::kSize; ++i )
    {
        bytes[i] = static_cast<std::uint8_t>( randB >> ( 8 * ( 15 - i ) ) );
    }

    return ObjectId::fromBytes( bytes );
}

ObjectId newObjectId()
{
    static ObjectIdGenerator generator;

    return generator.next();
}

std::size_t ObjectIdHash::operator()( const ObjectId& id ) const noexcept
{
    std::uint64_t high = 0;
    std::uint64_t low  = 0;

    for( std::size_t i = 0; i < 8; ++i )
    {
        high = ( high << 8 ) | id.bytes()[i];
        low  = ( low << 8 ) | id.bytes()[i + 8];
    }

    // Combine the halves, then apply the splitmix64 finaliser so IDs that differ only in the counter spread across
    // buckets.
    std::uint64_t value = high ^ ( low + 0x9E3779B97F4A7C15ULL + ( high << 6 ) + ( high >> 2 ) );

    value = ( value ^ ( value >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
    value = ( value ^ ( value >> 27 ) ) * 0x94D049BB133111EBULL;
    value = value ^ ( value >> 31 );

    return static_cast<std::size_t>( value );
}

} // namespace app::core
