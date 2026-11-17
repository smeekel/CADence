// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/ExprEvalTests.cpp
/// @brief  Unit tests for expression evaluation, the built-in functions and unit checking (T-01, A-08).

#include "core/expr/Evaluator.h"

#include <gtest/gtest.h>

#include <format>
#include <functional>
#include <map>
#include <numbers>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using app::core::Dimension;
using app::core::kAngle;
using app::core::kDimensionless;
using app::core::kLength;
using app::core::Quantity;

namespace
{

struct Expected
{
    std::string_view text;
    double           value;
    Dimension        dimension;
};

void expectValue( const Expected& expected, const app::core::expr::ReferenceResolver& resolve = {} )
{
    const app::Result<Quantity> result = app::core::expr::evaluateExpression( expected.text, resolve );

    ASSERT_TRUE( result.has_value() ) << expected.text << ": " << ( result ? "" : result.error().message );
    EXPECT_NEAR( result.value_or( Quantity() ).value(), expected.value, 1e-12 ) << expected.text;
    EXPECT_EQ( result.value_or( Quantity() ).dimension(), expected.dimension ) << expected.text;
}

std::string errorOf( std::string_view text, const app::core::expr::ReferenceResolver& resolve = {} )
{
    const app::Result<Quantity> result = app::core::expr::evaluateExpression( text, resolve );

    return result.has_value() ? "no error" : result.error().message;
}

constexpr double kPi = std::numbers::pi;

} // namespace

TEST( ExprEval, ConvertsUnitLiterals_WhenEvaluatingArithmetic )
{
    const std::vector<Expected> cases = {
        { "10 mm + 0.5 in", 22.7, kLength },
        { "2 * 3 mm", 6.0, kLength },
        { "(2 mm) ^ 2", 4.0, { 2, 0 } },
        { "(4 mm * 1 mm) ^ 0.5", 2.0, kLength },
        { "1 m / 1 mm", 1000.0, kDimensionless },
        { "1 / 3 mm", 1.0 / 3.0, { -1, 0 } },
        { "90 deg", kPi / 2.0, kAngle },
        { "-(3 mm) + +1 mm", -2.0, kLength },
        { "2 ^ 3 ^ 2", 512.0, kDimensionless },
        { "-2 ^ 2", -4.0, kDimensionless },
        { "2 ^ -1", 0.5, kDimensionless },
        { "25 um + 1 mil", 0.0504, kLength },
    };

    for( const Expected& expected : cases )
    {
        expectValue( expected );
    }
}

// Accept: A-08
TEST( ExprEval, ReportsUnitError_WhenDimensionsDoNotMatch )
{
    EXPECT_EQ( errorOf( "1 mm + 1 deg" ), "Cannot add length and angle at column 6." );
    EXPECT_EQ( errorOf( "1 mm - 1" ), "Cannot subtract dimensionless from length at column 6." );
    EXPECT_EQ( errorOf( "1 mm < 1 deg" ), "Cannot compare length and angle at column 6." );
    EXPECT_EQ( errorOf( "2 ^ 1 mm" ), "The exponent must be a number, not length at column 3." );
    EXPECT_EQ( errorOf( "(2 mm) ^ 0.5" ), "Cannot raise length to the power 0.5 at column 8." );

    const app::Result<Quantity> result = app::core::expr::evaluateExpression( "1 mm + 1 deg" );

    ASSERT_FALSE( result.has_value() );
    EXPECT_EQ( result.error().code, app::ErrorCode::InvalidArgument );
    EXPECT_EQ( result.error().detail, "1 mm + 1 deg\n     ^" );
}

TEST( ExprEval, ComparesWithinTolerance_WhenValuesAreClose )
{
    const std::vector<Expected> cases = {
        { "1 mm == 1.00000001 mm", 1.0, kDimensionless },
        { "1 mm == 1.001 mm", 0.0, kDimensionless },
        { "1 mm < 1.00000001 mm", 0.0, kDimensionless },
        { "1 mm <= 1 mm", 1.0, kDimensionless },
        { "2 > 1", 1.0, kDimensionless },
        { "1 mm != 1 mm", 0.0, kDimensionless },
        { "1 in >= 25.4 mm", 1.0, kDimensionless },
        { "1 in > 25.4 mm", 0.0, kDimensionless },
    };

    for( const Expected& expected : cases )
    {
        expectValue( expected );
    }
}

// Accept: A-08
TEST( ExprEval, EvaluatesEveryFunction_WhenArgumentsAreValid )
{
    const std::vector<Expected> cases = {
        { "sin(30 deg)", 0.5, kDimensionless },
        { "cos(60 deg)", 0.5, kDimensionless },
        { "tan(45 deg)", 1.0, kDimensionless },
        { "asin(0.5)", kPi / 6.0, kAngle },
        { "acos(0.5)", kPi / 3.0, kAngle },
        { "atan(1)", kPi / 4.0, kAngle },
        { "atan2(1 mm, 1 mm)", kPi / 4.0, kAngle },
        { "sqrt(16)", 4.0, kDimensionless },
        { "sqrt((3 mm) ^ 2)", 3.0, kLength },
        { "abs(-2 mm)", 2.0, kLength },
        { "min(3 mm, 1 mm, 2 mm)", 1.0, kLength },
        { "max(1, 5, 3)", 5.0, kDimensionless },
        { "floor(2.7)", 2.0, kDimensionless },
        { "ceil(2.1)", 3.0, kDimensionless },
        { "round(2.5)", 3.0, kDimensionless },
        { "round(-2.5)", -3.0, kDimensionless },
        { "if(1 < 2, 5 mm, 6 mm)", 5.0, kLength },
        { "if(0, 1, 2)", 2.0, kDimensionless },
    };

    for( const Expected& expected : cases )
    {
        expectValue( expected );
    }
}

TEST( ExprEval, TreatsPlainNumbersAsRadians_WhenPassedToTrigonometry )
{
    expectValue( { "sin(1.5707963267948966)", 1.0, kDimensionless } );
    expectValue( { "cos(0)", 1.0, kDimensionless } );
    expectValue( { "asin(1) == 90 deg", 1.0, kDimensionless } );
    expectValue( { "atan2(1, 1) / 1 deg", 45.0, kDimensionless } );
    EXPECT_EQ( errorOf( "sin(1 mm)" ), "sin needs an angle or a number, not length at column 1." );
}

TEST( ExprEval, ReportsColumn_WhenFunctionOrOperatorCannotApply )
{
    const std::vector<std::pair<std::string_view, std::string_view>> cases = {
        { "foo(1)", "Unknown function 'foo' at column 1." },
        { "1 + atan2(1)", "atan2 takes 2 arguments, not 1 at column 5." },
        { "min()", "min takes at least 1 argument at column 1." },
        { "if(1, 2)", "if takes 3 arguments, not 2 at column 1." },
        { "sqrt(-1)", "sqrt of a negative number at column 1." },
        { "sqrt(2 mm)", "sqrt needs an even power of each dimension, not length at column 1." },
        { "round(2.5 mm)",
          "round needs a number, not length; divide by a unit first, for example round(x / 1 mm) * 1 mm at column 1." },
        { "asin(2)", "asin needs a number from -1 to 1 at column 1." },
        { "atan(1 mm)", "atan needs a number, not length at column 1." },
        { "acos(1 deg)", "acos needs a number, not angle at column 1." },
        { "atan2(1 mm, 1 deg)", "atan2 needs arguments of the same dimension, not length and angle at column 1." },
        { "max(1 mm, 2)", "max needs arguments of the same dimension, not length and dimensionless at column 1." },
        { "if(1 mm, 1, 2)", "if needs a number as its condition, not length at column 4." },
        { "1 / (2 - 2)", "Division by zero at column 3." },
        { "(-8) ^ (1 / 3)", "A negative number cannot be raised to a fractional power at column 6." },
        { "1e300 * 1e300", "The result is not a finite number at column 7." },
        { "0 ^ -1", "The power is not a finite number at column 3." },
    };

    for( const auto& [text, message] : cases )
    {
        EXPECT_EQ( errorOf( text ), message ) << text;
    }
}

TEST( ExprEval, EvaluatesOnlyTheChosenBranch_WhenUsingIf )
{
    expectValue( { "if(1 < 2, 5 mm, sqrt(-1))", 5.0, kLength } );
    expectValue( { "if(2 < 1, sqrt(-1), 1)", 1.0, kDimensionless } );
}

TEST( ExprEval, ResolvesReferences_WhenResolverKnowsThem )
{
    const std::map<std::string, Quantity, std::less<>> parameters = {
        { "Board.Width", Quantity::length( 100.0 ) },
        { "t", Quantity::length( 1.6 ) },
    };

    const app::core::expr::ReferenceResolver resolve = [&]( std::string_view name ) -> app::Result<Quantity>
    {
        const auto found = parameters.find( name );

        if( found == parameters.end() )
        {
            return app::fail( app::ErrorCode::NotFound, std::format( "Unknown parameter '{}'.", name ) );
        }

        return found->second;
    };

    expectValue( { "Board.Width / 2 + t", 51.6, kLength }, resolve );

    const app::Result<Quantity> missing = app::core::expr::evaluateExpression( "2 * Board.Height", resolve );

    ASSERT_FALSE( missing.has_value() );
    EXPECT_EQ( missing.error().code, app::ErrorCode::NotFound );
    EXPECT_EQ( missing.error().message, "Unknown parameter 'Board.Height' at column 5." );
    EXPECT_EQ( errorOf( "x + 1" ), "Unknown parameter 'x' at column 1." );
}
