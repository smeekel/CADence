// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Functions.cpp
/// @brief  Built-in functions of the expression language.

#include "core/expr/Functions.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <limits>
#include <utility>

namespace app::core::expr
{

namespace
{

constexpr std::size_t kUnlimited = std::numeric_limits<std::size_t>::max();

using Implementation = Evaluated ( * )( std::span<const Quantity> );

struct Function
{
    std::string_view name;
    std::size_t      minArguments;
    std::size_t      maxArguments;
    Implementation   apply; // null for `if`
};

[[nodiscard]] std::unexpected<std::string> problem( std::string message )
{
    return std::unexpected( std::move( message ) );
}

[[nodiscard]] Evaluated finite( std::string_view name, const Quantity& result )
{
    if( !std::isfinite( result.value() ) )
    {
        return problem( std::format( "{} result is not a finite number", name ) );
    }

    return result;
}

// sin, cos and tan take an angle, or a plain number read as radians.
template <double ( *F )( double )>
Evaluated trigonometric( std::string_view name, std::span<const Quantity> arguments )
{
    const Quantity& x = arguments[0];

    if( x.dimension() != kAngle && !x.dimension().isDimensionless() )
    {
        return problem( std::format( "{} needs an angle or a number, not {}", name, describe( x.dimension() ) ) );
    }

    return finite( name, Quantity::number( F( x.value() ) ) );
}

// asin and acos take a number from -1 to 1 and return an angle.
template <double ( *F )( double )>
Evaluated inverseTrigonometric( std::string_view name, std::span<const Quantity> arguments )
{
    const Quantity& x = arguments[0];

    if( !x.dimension().isDimensionless() )
    {
        return problem( std::format( "{} needs a number, not {}", name, describe( x.dimension() ) ) );
    }

    if( x.value() < -1.0 || x.value() > 1.0 )
    {
        return problem( std::format( "{} needs a number from -1 to 1", name ) );
    }

    return Quantity::angle( F( x.value() ) );
}

[[nodiscard]] std::optional<std::string> sameDimension( std::string_view name, std::span<const Quantity> arguments )
{
    for( const Quantity& argument : arguments )
    {
        if( argument.dimension() != arguments[0].dimension() )
        {
            return std::format( "{} needs arguments of the same dimension, not {} and {}",
                                name,
                                describe( arguments[0].dimension() ),
                                describe( argument.dimension() ) );
        }
    }

    return std::nullopt;
}

// floor, ceil and round work on pure numbers only: rounding the internal millimetres of a value typed in inches
// would surprise the user, so they divide by a unit first.
template <double ( *F )( double )>
Evaluated rounding( std::string_view name, std::span<const Quantity> arguments )
{
    const Quantity& x = arguments[0];

    if( !x.dimension().isDimensionless() )
    {
        return problem(
            std::format( "{} needs a number, not {}; divide by a unit first, for example {}(x / 1 mm) * 1 mm",
                         name,
                         describe( x.dimension() ),
                         name ) );
    }

    return Quantity::number( F( x.value() ) );
}

[[nodiscard]] double sinOf( double x )
{
    return std::sin( x );
}

[[nodiscard]] double cosOf( double x )
{
    return std::cos( x );
}

[[nodiscard]] double tanOf( double x )
{
    return std::tan( x );
}

[[nodiscard]] double asinOf( double x )
{
    return std::asin( x );
}

[[nodiscard]] double acosOf( double x )
{
    return std::acos( x );
}

[[nodiscard]] double floorOf( double x )
{
    return std::floor( x );
}

[[nodiscard]] double ceilOf( double x )
{
    return std::ceil( x );
}

// Halves are rounded away from zero.
[[nodiscard]] double roundOf( double x )
{
    return std::round( x );
}

Evaluated atanFunction( std::span<const Quantity> arguments )
{
    const Quantity& x = arguments[0];

    if( !x.dimension().isDimensionless() )
    {
        return problem( std::format( "atan needs a number, not {}", describe( x.dimension() ) ) );
    }

    return Quantity::angle( std::atan( x.value() ) );
}

Evaluated atan2Function( std::span<const Quantity> arguments )
{
    if( auto mismatch = sameDimension( "atan2", arguments ) )
    {
        return problem( std::move( *mismatch ) );
    }

    return Quantity::angle( std::atan2( arguments[0].value(), arguments[1].value() ) );
}

Evaluated sqrtFunction( std::span<const Quantity> arguments )
{
    const Quantity&  x         = arguments[0];
    const Dimension& dimension = x.dimension();

    if( dimension.length % 2 != 0 || dimension.angle % 2 != 0 )
    {
        return problem( std::format( "sqrt needs an even power of each dimension, not {}", describe( dimension ) ) );
    }

    if( x.value() < 0.0 )
    {
        return problem( "sqrt of a negative number" );
    }

    return Quantity( std::sqrt( x.value() ), { dimension.length / 2, dimension.angle / 2 } );
}

Evaluated absFunction( std::span<const Quantity> arguments )
{
    return Quantity( std::abs( arguments[0].value() ), arguments[0].dimension() );
}

template <bool Max>
Evaluated extremum( std::span<const Quantity> arguments )
{
    constexpr std::string_view name = Max ? "max" : "min";

    if( auto mismatch = sameDimension( name, arguments ) )
    {
        return problem( std::move( *mismatch ) );
    }

    const auto chosen = std::ranges::min_element( arguments,
                                                  []( const Quantity& a, const Quantity& b )
                                                  {
                                                      return Max ? a.value() > b.value() : a.value() < b.value();
                                                  } );

    return *chosen;
}

constexpr std::array<Function, 15> kFunctions = { {
    { "sin",
      1,
      1,
      []( std::span<const Quantity> a )
      {
          return trigonometric<sinOf>( "sin", a );
      } },
    { "cos",
      1,
      1,
      []( std::span<const Quantity> a )
      {
          return trigonometric<cosOf>( "cos", a );
      } },
    { "tan",
      1,
      1,
      []( std::span<const Quantity> a )
      {
          return trigonometric<tanOf>( "tan", a );
      } },
    { "asin",
      1,
      1,
      []( std::span<const Quantity> a )
      {
          return inverseTrigonometric<asinOf>( "asin", a );
      } },
    { "acos",
      1,
      1,
      []( std::span<const Quantity> a )
      {
          return inverseTrigonometric<acosOf>( "acos", a );
      } },
    { "atan", 1, 1, atanFunction },
    { "atan2", 2, 2, atan2Function },
    { "sqrt", 1, 1, sqrtFunction },
    { "abs", 1, 1, absFunction },
    { "min", 1, kUnlimited, extremum<false> },
    { "max", 1, kUnlimited, extremum<true> },
    { "floor",
      1,
      1,
      []( std::span<const Quantity> a )
      {
          return rounding<floorOf>( "floor", a );
      } },
    { "ceil",
      1,
      1,
      []( std::span<const Quantity> a )
      {
          return rounding<ceilOf>( "ceil", a );
      } },
    { "round",
      1,
      1,
      []( std::span<const Quantity> a )
      {
          return rounding<roundOf>( "round", a );
      } },
    { "if", 3, 3, nullptr },
} };

[[nodiscard]] const Function* find( std::string_view name ) noexcept
{
    const auto match = std::ranges::find( kFunctions, name, &Function::name );

    return match == kFunctions.end() ? nullptr : &*match;
}

[[nodiscard]] std::string argumentCount( std::size_t count )
{
    return std::format( "{} argument{}", count, count == 1 ? "" : "s" );
}

} // namespace

bool isFunction( std::string_view name ) noexcept
{
    return find( name ) != nullptr;
}

std::optional<std::string> checkArgumentCount( std::string_view name, std::size_t count )
{
    const Function* function = find( name );

    if( function == nullptr || ( count >= function->minArguments && count <= function->maxArguments ) )
    {
        return std::nullopt;
    }

    if( function->maxArguments == kUnlimited )
    {
        return std::format( "{} takes at least {}", name, argumentCount( function->minArguments ) );
    }

    return std::format( "{} takes {}, not {}", name, argumentCount( function->minArguments ), count );
}

Evaluated applyFunction( std::string_view name, std::span<const Quantity> arguments )
{
    const Function* function = find( name );

    if( function == nullptr || function->apply == nullptr )
    {
        return problem( std::format( "{} cannot be applied to evaluated arguments", name ) );
    }

    return function->apply( arguments );
}

} // namespace app::core::expr
