// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/ExprDepsTests.cpp
/// @brief  Unit tests for expression dependency extraction and parameter scopes (T-01, A-08, A-07).

#include "core/expr/Dependencies.h"
#include "core/expr/Evaluator.h"
#include "core/expr/ParameterScope.h"
#include "core/expr/Parser.h"

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

using app::core::Quantity;
using app::core::expr::ParameterScope;

namespace
{

std::vector<std::string> namesIn( std::string_view text )
{
    const app::Result<app::core::expr::NodePtr> root = app::core::expr::parseExpression( text );

    if( !root.has_value() )
    {
        return { "parse error: " + root.error().message };
    }

    return app::core::expr::referencedNames( **root );
}

void define( ParameterScope& scope, std::string_view name, std::string_view expression )
{
    const app::Result<> defined = scope.define( name, expression );

    ASSERT_TRUE( defined.has_value() ) << name << ": " << ( defined ? "" : defined.error().message );
}

double millimetres( const app::core::expr::IParameterScope& scope, std::string_view expression )
{
    const app::Result<Quantity> value =
        app::core::expr::evaluateExpression( expression, app::core::expr::resolverFor( scope ) );

    EXPECT_TRUE( value.has_value() ) << expression << ": " << ( value ? "" : value.error().message );
    EXPECT_EQ( value.value_or( Quantity() ).dimension(), app::core::kLength ) << expression;
    return value.value_or( Quantity() ).value();
}

std::string errorOf( const app::core::expr::IParameterScope& scope, std::string_view expression )
{
    const app::Result<Quantity> value =
        app::core::expr::evaluateExpression( expression, app::core::expr::resolverFor( scope ) );

    return value.has_value() ? "no error" : value.error().message;
}

} // namespace

TEST( ExprDeps, ListsEachReferenceOnce_InOrderOfFirstAppearance )
{
    EXPECT_EQ( namesIn( "Board.Width + t * max(t, h) - Board.Width" ),
               ( std::vector<std::string>{ "Board.Width", "t", "h" } ) );
    EXPECT_EQ( namesIn( "if(a > 0, b, -c)" ), ( std::vector<std::string>{ "a", "b", "c" } ) );
    EXPECT_TRUE( namesIn( "sin(30 deg) * 2 mm" ).empty() );
}

TEST( ExprDeps, ResolvesNearestScopeFirst_WhenScopesAreNested )
{
    ParameterScope project( "project" );
    ParameterScope part( "part", &project );

    define( project, "Board.Width", "100 mm" );
    define( project, "t", "1.6 mm" );
    define( project, "outer", "t * 10" );
    define( part, "t", "2 mm" );
    define( part, "inner", "t * 10" );

    EXPECT_DOUBLE_EQ( millimetres( part, "t" ), 2.0 );
    EXPECT_DOUBLE_EQ( millimetres( project, "t" ), 1.6 );
    EXPECT_DOUBLE_EQ( millimetres( part, "Board.Width / 2" ), 50.0 );
    EXPECT_DOUBLE_EQ( millimetres( part, "inner" ), 20.0 );

    // A parameter's references resolve from the scope that defines it, not from where it is used.
    EXPECT_DOUBLE_EQ( millimetres( part, "outer" ), 16.0 );

    const app::Result<Quantity> missing = app::core::expr::lookup( part, "depth" );

    ASSERT_FALSE( missing.has_value() );
    EXPECT_EQ( missing.error().code, app::ErrorCode::NotFound );
    EXPECT_EQ( missing.error().message, "Unknown parameter 'depth'." );
    EXPECT_EQ( errorOf( part, "1 mm + depth" ), "Unknown parameter 'depth' at column 8." );
}

TEST( ExprDeps, EvaluatesChains_WhenParametersReferToEachOther )
{
    ParameterScope scope( "part" );

    define( scope, "a", "10 mm" );
    define( scope, "b", "a * 2" );
    define( scope, "c", "b + a" );

    EXPECT_DOUBLE_EQ( millimetres( scope, "c" ), 30.0 );

    define( scope, "a", "1 mm" );

    EXPECT_DOUBLE_EQ( millimetres( scope, "c" ), 3.0 );
    EXPECT_EQ( scope.dependenciesOf( "c" ),
               app::Result<std::vector<std::string>>( std::vector<std::string>{ "b", "a" } ) );
    EXPECT_EQ( scope.dependenciesOf( "zz" ).error().code, app::ErrorCode::NotFound );
}

// Accept: A-08
TEST( ExprDeps, NamesEveryParameterInTheCycle_WhenParametersFormACycle )
{
    ParameterScope scope( "part" );

    define( scope, "a", "b + 1 mm" );
    define( scope, "b", "c * 2" );
    define( scope, "c", "a - 1 mm" );
    define( scope, "self", "self" );

    const app::Result<Quantity> cycle = app::core::expr::lookup( scope, "a" );

    ASSERT_FALSE( cycle.has_value() );
    EXPECT_EQ( cycle.error().code, app::ErrorCode::Conflict );
    EXPECT_EQ( cycle.error().message, "Parameter cycle in part: a -> b -> c -> a." );
    EXPECT_EQ( errorOf( scope, "2 * b" ), "Parameter cycle in part: b -> c -> a -> b." );
    EXPECT_EQ( errorOf( scope, "self" ), "Parameter cycle in part: self -> self." );

    define( scope, "c", "5 mm" );

    EXPECT_DOUBLE_EQ( millimetres( scope, "a" ), 11.0 );
}

TEST( ExprDeps, PrefixesParameterName_WhenItsExpressionIsInvalid )
{
    ParameterScope scope( "part" );

    define( scope, "a", "1 mm + 1 deg" );
    define( scope, "b", "q + 1" );
    define( scope, "c", "a * 2" );

    EXPECT_EQ( errorOf( scope, "a * 2" ), "Parameter 'a': Cannot add length and angle at column 6." );
    EXPECT_EQ( errorOf( scope, "b" ), "Parameter 'b': Unknown parameter 'q' at column 1." );
    EXPECT_EQ( errorOf( scope, "c + 1 mm" ), "Parameter 'a': Cannot add length and angle at column 6." );

    const app::Result<Quantity> invalid = app::core::expr::lookup( scope, "b" );

    ASSERT_FALSE( invalid.has_value() );
    EXPECT_EQ( invalid.error().code, app::ErrorCode::InvalidArgument );
}

TEST( ExprDeps, ValidatesNamesAndExpressions_WhenDefiningParameters )
{
    ParameterScope scope( "project" );

    for( const std::string_view name : { "2x", "", "a b", "a..b", "a.", ".a", "a+b", "Board. Width" } )
    {
        const app::Result<> defined = scope.define( name, "1" );

        ASSERT_FALSE( defined.has_value() ) << name;
        EXPECT_EQ( defined.error().message, std::string( "Invalid parameter name '" ) + std::string( name ) + "'." );
    }

    const app::Result<> syntax = scope.define( "a", "(1" );

    ASSERT_FALSE( syntax.has_value() );
    EXPECT_EQ( syntax.error().message, "Expected ')' at column 3." );
    EXPECT_TRUE( scope.names().empty() );

    define( scope, "Board.Width", "100 mm" );
    define( scope, "_n2", "3" );

    EXPECT_EQ( scope.names(), ( std::vector<std::string>{ "Board.Width", "_n2" } ) );
    EXPECT_EQ( scope.expressionOf( "Board.Width" ), std::optional<std::string>( "100 mm" ) );
    EXPECT_TRUE( scope.remove( "_n2" ) );
    EXPECT_FALSE( scope.remove( "_n2" ) );
    EXPECT_FALSE( scope.expressionOf( "_n2" ).has_value() );
    EXPECT_EQ( scope.scopeName(), "project" );
    EXPECT_EQ( scope.parent(), nullptr );
}
