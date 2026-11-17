// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/UnitsTests.cpp
/// @brief  Unit tests for units, quantities and tolerances (T-01, P-06, P-07).

#include "core/Quantity.h"
#include "core/Tolerance.h"
#include "core/Units.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>
#include <optional>
#include <string>
#include <string_view>

using app::core::Quantity;
using app::core::Unit;

namespace
{

constexpr Unit kAllUnits[] = { Unit::Millimetre,
                               Unit::Centimetre,
                               Unit::Metre,
                               Unit::Inch,
                               Unit::Mil,
                               Unit::Thou,
                               Unit::Micrometre,
                               Unit::Radian,
                               Unit::Degree };

constexpr Unit kLengthUnits[] =
    { Unit::Millimetre, Unit::Centimetre, Unit::Metre, Unit::Inch, Unit::Mil, Unit::Thou, Unit::Micrometre };

// Deterministic pseudo-random doubles in [-range, range] (splitmix64), so the round-trip tests need no seeded engine.
double nextValue( std::uint64_t& state, double range )
{
    state += 0x9E3779B97F4A7C15ULL;

    std::uint64_t z = state;

    z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
    z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBULL;
    z = z ^ ( z >> 31 );

    const double unit = static_cast<double>( z >> 11 ) / static_cast<double>( std::uint64_t{ 1 } << 53 );

    return ( unit * 2.0 - 1.0 ) * range;
}

} // namespace

// Accept: P-06
TEST( Units, ConvertsWithExactFactors_WhenValuesAreWholeNumbers )
{
    EXPECT_EQ( app::core::toInternal( 1.0, Unit::Inch ), 25.4 );
    EXPECT_EQ( app::core::toInternal( 1000.0, Unit::Mil ), 25.4 );
    EXPECT_EQ( app::core::toInternal( 1000.0, Unit::Thou ), 25.4 );
    EXPECT_EQ( app::core::toInternal( 1.0, Unit::Mil ), 0.0254 );
    EXPECT_EQ( app::core::toInternal( 127.0, Unit::Inch ), 3225.8 );
    EXPECT_EQ( app::core::toInternal( 1.0, Unit::Centimetre ), 10.0 );
    EXPECT_EQ( app::core::toInternal( 1.0, Unit::Metre ), 1000.0 );
    EXPECT_EQ( app::core::toInternal( 1.0, Unit::Micrometre ), 0.001 );
    EXPECT_EQ( app::core::toInternal( 7.0, Unit::Millimetre ), 7.0 );
    EXPECT_EQ( app::core::toInternal( 2.0, Unit::Radian ), 2.0 );

    EXPECT_EQ( app::core::fromInternal( 25.4, Unit::Inch ), 1.0 );
    EXPECT_EQ( app::core::fromInternal( 25.4, Unit::Mil ), 1000.0 );
    EXPECT_EQ( app::core::fromInternal( 1.0, Unit::Micrometre ), 1000.0 );
    EXPECT_EQ( app::core::fromInternal( 1000.0, Unit::Metre ), 1.0 );
}

TEST( Units, ConvertsAnglesThroughPi_WhenUnitIsDegree )
{
    EXPECT_DOUBLE_EQ( app::core::toInternal( 180.0, Unit::Degree ), std::numbers::pi );
    EXPECT_DOUBLE_EQ( app::core::toInternal( 90.0, Unit::Degree ), std::numbers::pi / 2.0 );
    EXPECT_DOUBLE_EQ( app::core::fromInternal( std::numbers::pi, Unit::Degree ), 180.0 );
}

TEST( Units, ReportsDimensionAndSymbol_ForEveryUnit )
{
    for( Unit unit : kAllUnits )
    {
        const std::optional<Unit> parsed = app::core::parseUnit( app::core::symbol( unit ) );

        EXPECT_EQ( parsed, std::optional<Unit>( unit ) ) << app::core::symbol( unit );
    }

    EXPECT_EQ( app::core::dimensionOf( Unit::Inch ), app::core::kLength );
    EXPECT_EQ( app::core::dimensionOf( Unit::Degree ), app::core::kAngle );
    EXPECT_EQ( app::core::symbol( Unit::Micrometre ), "\xC2\xB5m" );
}

// Accept: P-06
TEST( Units, ParsesMicrometreSpellings_WhenGivenMicroSignGreekMuOrAscii )
{
    EXPECT_EQ( app::core::parseUnit( "\xC2\xB5m" ), std::optional<Unit>( Unit::Micrometre ) );
    EXPECT_EQ( app::core::parseUnit( "\xCE\xBCm" ), std::optional<Unit>( Unit::Micrometre ) );
    EXPECT_EQ( app::core::parseUnit( "um" ), std::optional<Unit>( Unit::Micrometre ) );
    EXPECT_FALSE( app::core::parseUnit( "MM" ).has_value() );
    EXPECT_FALSE( app::core::parseUnit( "inch" ).has_value() );
    EXPECT_FALSE( app::core::parseUnit( "" ).has_value() );
}

TEST( Units, ParsesQuantities_WhenTextHasNumberAndUnit )
{
    const auto lengthOf = []( std::string_view text )
    {
        return app::core::parseQuantity( text, Unit::Millimetre ).value_or( Quantity() );
    };

    EXPECT_EQ( lengthOf( "12.7 mm" ), Quantity::length( 12.7 ) );
    EXPECT_EQ( lengthOf( "0.5in" ), Quantity::length( 12.7 ) );
    EXPECT_EQ( lengthOf( "  -3 \xC2\xB5m " ), Quantity::length( -0.003 ) );
    EXPECT_EQ( lengthOf( "250 um" ), Quantity::length( 0.25 ) );
    EXPECT_EQ( lengthOf( "+2 cm" ), Quantity::length( 20.0 ) );
    EXPECT_EQ( lengthOf( "1e3 mil" ), Quantity::length( 25.4 ) );
    EXPECT_EQ( lengthOf( "4" ), Quantity::length( 4.0 ) );

    const app::Result<Quantity> angle = app::core::parseQuantity( "180 deg", Unit::Millimetre );

    ASSERT_TRUE( angle.has_value() );
    EXPECT_EQ( angle.value_or( Quantity() ).dimension(), app::core::kAngle );
    EXPECT_DOUBLE_EQ( angle.value_or( Quantity() ).value(), std::numbers::pi );

    EXPECT_EQ( app::core::parseQuantity( "30", Unit::Degree ).value_or( Quantity() ).dimension(), app::core::kAngle );
}

TEST( Units, RejectsQuantityText_WhenNumberOrUnitIsInvalid )
{
    const std::string_view invalid[] = { "",
                                         "   ",
                                         "mm",
                                         "12 furlong",
                                         "12 MM",
                                         "+-3 mm",
                                         "++3 mm",
                                         "nan mm",
                                         "inf",
                                         "1e400 mm",
                                         "12 mm mm",
                                         "1,5 mm" };

    for( std::string_view text : invalid )
    {
        const app::Result<Quantity> result = app::core::parseQuantity( text, Unit::Millimetre );

        ASSERT_FALSE( result.has_value() ) << '"' << text << '"';
        EXPECT_EQ( result.error().code, app::ErrorCode::InvalidArgument );
        EXPECT_EQ( result.error().detail, text );
    }

    EXPECT_EQ( app::core::parseQuantity( "12 furlong", Unit::Millimetre ).error().message, "Unknown unit 'furlong'." );
}

TEST( Units, FormatsWithDisplayUnitsAndDecimals_WhenGivenDocumentDefaults )
{
    const app::core::DisplayUnits defaults;

    EXPECT_EQ( app::core::formatQuantity( Quantity::length( 12.7 ), defaults ), "12.700 mm" );
    EXPECT_EQ( app::core::formatQuantity( Quantity::angle( std::numbers::pi / 4.0 ), defaults ), "45.00 deg" );
    EXPECT_EQ( app::core::formatQuantity( Quantity::number( 0.5 ), defaults ), "0.5" );
    EXPECT_EQ( app::core::formatQuantity( Quantity::length( -0.0001 ), defaults ), "0.000 mm" );

    app::core::DisplayUnits imperial;

    imperial.length         = Unit::Inch;
    imperial.lengthDecimals = 4;
    imperial.angle          = Unit::Radian;
    imperial.angleDecimals  = 3;

    EXPECT_EQ( app::core::formatQuantity( Quantity::length( 12.7 ), imperial ), "0.5000 in" );
    EXPECT_EQ( app::core::formatQuantity( Quantity::angle( 1.0 ), imperial ), "1.000 rad" );
    EXPECT_EQ( app::core::formatValue( 0.003, Unit::Micrometre, std::nullopt ), "3 \xC2\xB5m" );
}

TEST( Units, ComposesInternalSymbol_WhenDimensionIsNotLengthOrAngle )
{
    const app::core::DisplayUnits defaults;
    const Quantity                area = Quantity::length( 2.0 ) * Quantity::length( 3.0 );
    const Quantity                rate = Quantity::length( 6.0 ) / Quantity::angle( 2.0 );

    EXPECT_EQ( app::core::formatQuantity( area, defaults ), "6 mm^2" );
    EXPECT_EQ( app::core::formatQuantity( rate, defaults ), "3 mm/rad" );
    EXPECT_EQ( app::core::formatQuantity( Quantity::number( 1.0 ) / Quantity::length( 4.0 ), defaults ), "0.25 1/mm" );
}

// Accept: P-06
TEST( Units, RoundTripsDisplayedText_WhenFormattedAndParsedInEveryLengthUnit )
{
    std::uint64_t state = 2026;

    for( Unit unit : kLengthUnits )
    {
        for( int decimals = 0; decimals <= 6; ++decimals )
        {
            for( int i = 0; i < 500; ++i )
            {
                const double                internal = nextValue( state, 5000.0 );
                const std::string           shown    = app::core::formatValue( internal, unit, decimals );
                const app::Result<Quantity> parsed   = app::core::parseQuantity( shown, Unit::Millimetre );

                ASSERT_TRUE( parsed.has_value() ) << shown;
                ASSERT_EQ( app::core::formatValue( parsed.value_or( Quantity() ).value(), unit, decimals ), shown );
            }
        }
    }
}

TEST( Units, RoundTripsExactly_WhenShortestFormIsUsedInMillimetres )
{
    std::uint64_t state = 7;

    for( int i = 0; i < 10'000; ++i )
    {
        const double                value = nextValue( state, 1.0e6 );
        const app::Result<Quantity> parsed =
            app::core::parseQuantity( app::core::formatValue( value, Unit::Millimetre, std::nullopt ),
                                      Unit::Millimetre );

        ASSERT_EQ( parsed.value_or( Quantity() ), Quantity::length( value ) );
    }
}

TEST( Quantity, CombinesDimensions_WhenMultipliedOrDivided )
{
    const Quantity area = Quantity::length( 2.0 ) * Quantity::length( 5.0 );

    EXPECT_EQ( area, Quantity( 10.0, { 2, 0 } ) );
    EXPECT_EQ( area / Quantity::length( 5.0 ), Quantity::length( 2.0 ) );
    EXPECT_TRUE( ( Quantity::angle( 1.0 ) / Quantity::angle( 1.0 ) ).dimension().isDimensionless() );
    EXPECT_EQ( -Quantity::length( 3.0 ), Quantity::length( -3.0 ) );
}

// Accept: A-08 (unit checking: adding mm to degrees is an error)
TEST( Quantity, RejectsAddition_WhenDimensionsDiffer )
{
    const app::Result<Quantity> mixed = app::core::add( Quantity::length( 1.0 ), Quantity::angle( 1.0 ) );

    ASSERT_FALSE( mixed.has_value() );
    EXPECT_EQ( mixed.error().code, app::ErrorCode::InvalidArgument );
    EXPECT_EQ( mixed.error().message, "Cannot add length and angle." );

    const app::Result<Quantity> difference =
        app::core::subtract( Quantity::length( 1.0 ) * Quantity::length( 1.0 ), Quantity::length( 1.0 ) );

    ASSERT_FALSE( difference.has_value() );
    EXPECT_EQ( difference.error().message, "Cannot subtract length from length^2." );

    EXPECT_EQ( app::core::add( Quantity::length( 1.0 ), Quantity::length( 2.0 ) ).value_or( Quantity() ),
               Quantity::length( 3.0 ) );
    EXPECT_EQ( app::core::subtract( Quantity::angle( 3.0 ), Quantity::angle( 1.0 ) ).value_or( Quantity() ),
               Quantity::angle( 2.0 ) );
}

TEST( Quantity, DescribesDimensions_WhenNamedInMessages )
{
    EXPECT_EQ( app::core::describe( app::core::kDimensionless ), "dimensionless" );
    EXPECT_EQ( app::core::describe( app::core::kLength ), "length" );
    EXPECT_EQ( app::core::describe( { 3, 0 } ), "length^3" );
    EXPECT_EQ( app::core::describe( { 1, -1 } ), "length/angle" );
    EXPECT_EQ( app::core::describe( { -2, 0 } ), "1/length^2" );
    EXPECT_EQ( app::core::describe( { 1, 1 } ), "length*angle" );
}

// Accept: P-07
TEST( Tolerance, UsesOneTenthMicrometre_WhenComparingLengths )
{
    EXPECT_EQ( app::core::kConfusionTolerance, 1e-7 );
    EXPECT_TRUE( app::core::isEqualWithin( 10.0, 10.0 + 5e-8 ) );
    EXPECT_FALSE( app::core::isEqualWithin( 10.0, 10.0 + 2e-7 ) );
    EXPECT_TRUE( app::core::isEqualWithin( 1.0, 1.5, 0.5 ) );
    EXPECT_FALSE( app::core::isEqualWithin( std::numeric_limits<double>::quiet_NaN(), 0.0 ) );
}
