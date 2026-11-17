// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Units.cpp
/// @brief  Unit conversion, dimension arithmetic, and formatting and parsing of values with units.

#include "core/Units.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <format>
#include <numbers>
#include <system_error>
#include <utility>

namespace app::core
{

namespace
{

// Conversion to internal units is value * numerator / denominator. Keeping the ratio of exact integers (254 / 10
// rather than 25.4) makes whole numbers of a unit convert to the correctly rounded internal value.
struct Factor
{
    double numerator;
    double denominator;
};

constexpr Factor factorOf( Unit unit ) noexcept
{
    switch( unit )
    {
        case Unit::Millimetre:
            return { 1.0, 1.0 };
        case Unit::Centimetre:
            return { 10.0, 1.0 };
        case Unit::Metre:
            return { 1000.0, 1.0 };
        case Unit::Inch:
            return { 254.0, 10.0 };
        case Unit::Mil:
        case Unit::Thou:
            return { 254.0, 10000.0 };
        case Unit::Micrometre:
            return { 1.0, 1000.0 };
        case Unit::Radian:
            return { 1.0, 1.0 };
        case Unit::Degree:
            return { std::numbers::pi, 180.0 };
    }

    return { 1.0, 1.0 };
}

constexpr std::string_view kMicroSignMetre = "\xC2\xB5m"; // U+00B5 MICRO SIGN, then "m"
constexpr std::string_view kGreekMuMetre   = "\xCE\xBCm"; // U+03BC GREEK SMALL LETTER MU, then "m"

constexpr std::string_view kWhitespace = " \t";

std::string_view trim( std::string_view text ) noexcept
{
    const std::size_t first = text.find_first_not_of( kWhitespace );

    if( first == std::string_view::npos )
    {
        return {};
    }

    const std::size_t last = text.find_last_not_of( kWhitespace );

    return text.substr( first, last - first + 1 );
}

// Builds "a^2*b/c" from base names and exponents; `one` is used when there is no positive exponent.
std::string composeDimension( const Dimension& dimension,
                              std::string_view lengthName,
                              std::string_view angleName,
                              std::string_view one )
{
    const std::array<std::pair<std::string_view, int>, 2> bases = {
        { { lengthName, dimension.length }, { angleName, dimension.angle } } };

    const auto term = []( std::string_view name, int exponent )
    {
        return exponent == 1 ? std::string( name ) : std::format( "{}^{}", name, exponent );
    };

    std::string numerator;
    std::string denominator;

    for( const auto& [name, exponent] : bases )
    {
        if( exponent > 0 )
        {
            numerator += ( numerator.empty() ? "" : "*" ) + term( name, exponent );
        }
        else if( exponent < 0 )
        {
            denominator += "/" + term( name, -exponent );
        }
    }

    return ( numerator.empty() ? std::string( one ) : numerator ) + denominator;
}

} // namespace

std::string describe( const Dimension& dimension )
{
    if( dimension.isDimensionless() )
    {
        return "dimensionless";
    }

    return composeDimension( dimension, "length", "angle", "1" );
}

Result<Quantity> add( const Quantity& lhs, const Quantity& rhs )
{
    if( lhs.dimension() != rhs.dimension() )
    {
        return fail( ErrorCode::InvalidArgument,
                     std::format( "Cannot add {} and {}.", describe( lhs.dimension() ), describe( rhs.dimension() ) ) );
    }

    return Quantity( lhs.value() + rhs.value(), lhs.dimension() );
}

Result<Quantity> subtract( const Quantity& lhs, const Quantity& rhs )
{
    if( lhs.dimension() != rhs.dimension() )
    {
        return fail(
            ErrorCode::InvalidArgument,
            std::format( "Cannot subtract {} from {}.", describe( rhs.dimension() ), describe( lhs.dimension() ) ) );
    }

    return Quantity( lhs.value() - rhs.value(), lhs.dimension() );
}

Dimension dimensionOf( Unit unit ) noexcept
{
    return unit == Unit::Radian || unit == Unit::Degree ? kAngle : kLength;
}

std::string_view symbol( Unit unit ) noexcept
{
    switch( unit )
    {
        case Unit::Millimetre:
            return "mm";
        case Unit::Centimetre:
            return "cm";
        case Unit::Metre:
            return "m";
        case Unit::Inch:
            return "in";
        case Unit::Mil:
            return "mil";
        case Unit::Thou:
            return "thou";
        case Unit::Micrometre:
            return kMicroSignMetre;
        case Unit::Radian:
            return "rad";
        case Unit::Degree:
            return "deg";
    }

    return "?";
}

std::optional<Unit> parseUnit( std::string_view text ) noexcept
{
    static constexpr std::array<std::pair<std::string_view, Unit>, 11> kSymbols = { {
        { "mm", Unit::Millimetre },
        { "cm", Unit::Centimetre },
        { "m", Unit::Metre },
        { "in", Unit::Inch },
        { "mil", Unit::Mil },
        { "thou", Unit::Thou },
        { kMicroSignMetre, Unit::Micrometre },
        { kGreekMuMetre, Unit::Micrometre },
        { "um", Unit::Micrometre },
        { "rad", Unit::Radian },
        { "deg", Unit::Degree },
    } };

    for( const auto& [name, unit] : kSymbols )
    {
        if( name == text )
        {
            return unit;
        }
    }

    return std::nullopt;
}

double toInternal( double value, Unit unit ) noexcept
{
    const Factor factor = factorOf( unit );

    return value * factor.numerator / factor.denominator;
}

double fromInternal( double internal, Unit unit ) noexcept
{
    const Factor factor = factorOf( unit );

    return internal * factor.denominator / factor.numerator;
}

std::string formatNumber( double value, std::optional<int> decimals )
{
    // Fixed notation of the largest double with the most decimals allowed here fits in 309 + 1 + 17 characters.
    std::array<char, 400> buffer{};

    const std::to_chars_result result = decimals.has_value()
                                            ? std::to_chars( buffer.data(),
                                                             buffer.data() + buffer.size(),
                                                             value,
                                                             std::chars_format::fixed,
                                                             std::clamp( *decimals, 0, 17 ) )
                                            : std::to_chars( buffer.data(), buffer.data() + buffer.size(), value );

    std::string text( buffer.data(), result.ptr );

    if( text.starts_with( '-' ) && text.find_first_not_of( "-0." ) == std::string::npos )
    {
        text.erase( 0, 1 );
    }

    return text;
}

std::string formatValue( double internal, Unit unit, std::optional<int> decimals )
{
    return formatNumber( fromInternal( internal, unit ), decimals ) + " " + std::string( symbol( unit ) );
}

std::string formatQuantity( const Quantity& quantity, const DisplayUnits& display )
{
    const Dimension dimension = quantity.dimension();

    if( dimension == kLength )
    {
        return formatValue( quantity.value(), display.length, display.lengthDecimals );
    }

    if( dimension == kAngle )
    {
        return formatValue( quantity.value(), display.angle, display.angleDecimals );
    }

    if( dimension.isDimensionless() )
    {
        return formatNumber( quantity.value(), std::nullopt );
    }

    return formatNumber( quantity.value(), std::nullopt ) + " " + composeDimension( dimension, "mm", "rad", "1" );
}

Result<Quantity> parseQuantity( std::string_view text, Unit defaultUnit )
{
    const std::string_view trimmed = trim( text );
    std::string_view       number  = trimmed;

    // std::from_chars accepts a leading '-' but not '+'.
    if( number.starts_with( '+' ) )
    {
        number.remove_prefix( 1 );

        if( number.starts_with( '-' ) || number.starts_with( '+' ) )
        {
            return fail( ErrorCode::InvalidArgument, "Expected a number.", std::string( text ) );
        }
    }

    double                       value  = 0.0;
    const std::from_chars_result parsed = std::from_chars( number.data(), number.data() + number.size(), value );

    if( parsed.ec == std::errc::result_out_of_range )
    {
        return fail( ErrorCode::InvalidArgument, "Number is out of range.", std::string( text ) );
    }

    if( parsed.ec != std::errc{} || !std::isfinite( value ) )
    {
        return fail( ErrorCode::InvalidArgument, "Expected a number.", std::string( text ) );
    }

    const std::string_view unitText = trim( number.substr( static_cast<std::size_t>( parsed.ptr - number.data() ) ) );
    Unit                   unit     = defaultUnit;

    if( !unitText.empty() )
    {
        const std::optional<Unit> named = parseUnit( unitText );

        if( !named.has_value() )
        {
            return fail( ErrorCode::InvalidArgument,
                         std::format( "Unknown unit '{}'.", unitText ),
                         std::string( text ) );
        }

        unit = *named;
    }

    return Quantity( toInternal( value, unit ), dimensionOf( unit ) );
}

} // namespace app::core
