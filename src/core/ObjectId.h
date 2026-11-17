// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/ObjectId.h
/// @brief  Stable object identity: a time-ordered UUID version 7 (RFC 9562) and its generator.
/// @req    A-04

#pragma once

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace app::core
{

/// @brief Identity of a persistent object: a 128-bit UUID, normally version 7.
///
/// The 16 bytes are kept in RFC 4122 (network, big-endian) order, which is also the order of the text form and of
/// BSON binary subtype 0x04. Comparison is byte-wise, so version 7 IDs order by creation time. The default value is
/// the nil UUID, which never identifies an object.
///
/// @req A-04
class ObjectId
{
public:
    /// @brief Number of bytes in an ID.
    static constexpr std::size_t kSize = 16;

    /// @brief The 16 bytes of an ID in RFC 4122 order.
    using Bytes = std::array<std::uint8_t, kSize>;

    /// @brief Construct the nil UUID.
    constexpr ObjectId() noexcept = default;

    /// @brief  Build an ID from bytes in RFC 4122 order. Any UUID version is accepted.
    /// @param  bytes  The 16 bytes, most significant first.
    /// @return The ID holding @p bytes.
    [[nodiscard]] static constexpr ObjectId fromBytes( const Bytes& bytes ) noexcept
    {
        ObjectId id;

        id.m_bytes = bytes;
        return id;
    }

    /// @brief  Parse the canonical text form `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx` (36 characters, lower-case
    ///         hexadecimal digits; ADR 0028). Any UUID version is accepted.
    /// @param  text  Text to parse; no braces, prefix, upper case or surrounding whitespace.
    /// @return The ID, or `std::nullopt` when @p text is not in canonical form.
    [[nodiscard]] static std::optional<ObjectId> parse( std::string_view text ) noexcept;

    /// @brief  The bytes in RFC 4122 order.
    /// @return The 16 bytes, most significant first.
    [[nodiscard]] constexpr const Bytes& bytes() const noexcept
    {
        return m_bytes;
    }

    /// @brief  Canonical text form, lowercase and hyphenated, as stored in JSON.
    /// @return 36-character string.
    [[nodiscard]] std::string toString() const;

    /// @brief  UUID version from the high nibble of byte 6 (7 for generated IDs, 0 for nil).
    /// @return Version number 0 to 15.
    [[nodiscard]] constexpr int version() const noexcept
    {
        return m_bytes[6] >> 4;
    }

    /// @brief  Whether the variant bits are the RFC 4122 / RFC 9562 variant (binary `10`).
    /// @return True for generated IDs.
    [[nodiscard]] constexpr bool hasRfcVariant() const noexcept
    {
        return ( m_bytes[8] & 0xC0 ) == 0x80;
    }

    /// @brief  Whether this is the nil UUID (all bits zero).
    /// @return True for a default-constructed ID.
    [[nodiscard]] constexpr bool isNil() const noexcept
    {
        return m_bytes == Bytes{};
    }

    /// @brief  Unix timestamp held by a version 7 ID.
    /// @return Milliseconds since 1970-01-01 UTC from the first 48 bits. @units ms
    [[nodiscard]] constexpr std::uint64_t unixMilliseconds() const noexcept
    {
        std::uint64_t value = 0;

        for( std::size_t i = 0; i < 6; ++i )
        {
            value = ( value << 8 ) | m_bytes[i];
        }

        return value;
    }

    /// @brief IDs compare byte-wise, which orders version 7 IDs by creation time.
    [[nodiscard]] friend constexpr auto operator<=>( const ObjectId&, const ObjectId& ) noexcept = default;

    /// @brief IDs are equal when all 16 bytes are equal.
    [[nodiscard]] friend constexpr bool operator==( const ObjectId&, const ObjectId& ) noexcept = default;

private:
    Bytes m_bytes{};
};

/// @brief Generates version 7 IDs that are unique and strictly increasing for one generator.
///
/// Layout (RFC 9562, section 6.2, method 1): 48-bit Unix millisecond timestamp, version, a 42-bit counter in `rand_a`
/// and the top 30 bits of `rand_b`, variant, then 32 random bits. The counter is seeded with 41 random bits for each
/// new millisecond and incremented while the clock stands still or goes backwards; when it overflows, the timestamp
/// moves one millisecond ahead. `next()` is thread-safe.
///
/// @req A-04
class ObjectIdGenerator
{
public:
    /// @brief Clock returning the current Unix time. @units ms
    using Clock = std::function<std::uint64_t()>;

    /// @brief Source of 64 random bits per call.
    using RandomSource = std::function<std::uint64_t()>;

    /// @brief Generator using the system clock and a 64-bit Mersenne Twister seeded from `std::random_device`.
    ObjectIdGenerator();

    /// @brief Generator with an injected clock and random source.
    /// @param[in] clock   Current Unix time in milliseconds; values above 48 bits are truncated.
    /// @param[in] random  64 random bits per call.
    ObjectIdGenerator( Clock clock, RandomSource random );

    /// @brief  Reproducible generator for tests: a fixed clock and a Mersenne Twister with a fixed seed. Two generators
    ///         built with the same arguments produce the same sequence.
    /// @param  seed      Random seed.
    /// @param  unixMs    Time the clock always returns. @units ms
    /// @return The generator.
    [[nodiscard]] static ObjectIdGenerator makeDeterministic( std::uint64_t seed, std::uint64_t unixMs );

    /// @brief  Generate the next ID; greater than every ID this generator returned before.
    /// @return A version 7 ID with the RFC variant.
    [[nodiscard]] ObjectId next();

private:
    Clock         m_clock;
    RandomSource  m_random;
    std::mutex    m_mutex;
    std::uint64_t m_lastMs  = 0;
    std::uint64_t m_counter = 0;
    bool          m_started = false;
};

/// @brief  Generate an ID with the process-wide generator (system clock, random seed).
/// @return A new version 7 ID.
[[nodiscard]] ObjectId newObjectId();

/// @brief Hash of an ObjectId; `std::hash<ObjectId>` uses it, so IDs work as keys of unordered containers.
struct ObjectIdHash
{
    /// @brief  Hash of the 16 bytes.
    /// @param  id  ID to hash.
    /// @return Hash value; equal IDs hash equally.
    [[nodiscard]] std::size_t operator()( const ObjectId& id ) const noexcept;
};

} // namespace app::core

/// @cond
template <>
struct std::hash<app::core::ObjectId> : app::core::ObjectIdHash
{
};

/// @endcond
