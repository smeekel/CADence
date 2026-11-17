// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/ExprParserTests.cpp
/// @brief  Unit tests for the expression lexer and parser (T-01, A-08).

#include "core/expr/Lexer.h"
#include "core/expr/Parser.h"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

using app::core::expr::Binary;
using app::core::expr::SourceRange;
using app::core::expr::TokenKind;

namespace
{

// The parenthesised form of a successful parse, or the error message.
std::string parsed( std::string_view text )
{
    const app::Result<app::core::expr::NodePtr> root = app::core::expr::parseExpression( text );

    if( !root.has_value() )
    {
        return "error: " + root.error().message;
    }

    return app::core::expr::toText( **root );
}

std::string repeated( std::string_view text, int count )
{
    std::string result;

    for( int i = 0; i < count; ++i )
    {
        result += text;
    }

    return result;
}

} // namespace

TEST( ExprParser, AppliesPrecedenceAndLeftAssociativity_WhenOperatorsMix )
{
    EXPECT_EQ( parsed( "1 + 2 * 3" ), "(1 + (2 * 3))" );
    EXPECT_EQ( parsed( "1 - 2 - 3" ), "((1 - 2) - 3)" );
    EXPECT_EQ( parsed( "8 / 4 / 2" ), "((8 / 4) / 2)" );
    EXPECT_EQ( parsed( "(1 + 2) * 3" ), "((1 + 2) * 3)" );
    EXPECT_EQ( parsed( "-a * b" ), "((-a) * b)" );
    EXPECT_EQ( parsed( "a < b + 1" ), "(a < (b + 1))" );
    EXPECT_EQ( parsed( "a + 1 >= b * 2" ), "((a + 1) >= (b * 2))" );
    EXPECT_EQ( parsed( "a==b" ), "(a == b)" );
    EXPECT_EQ( parsed( "a != b" ), "(a != b)" );
}

TEST( ExprParser, GroupsPowerFromTheRight_WhenChained )
{
    EXPECT_EQ( parsed( "2 ^ 3 ^ 2" ), "(2 ^ (3 ^ 2))" );
    EXPECT_EQ( parsed( "-2 ^ 2" ), "(-(2 ^ 2))" );
    EXPECT_EQ( parsed( "2 ^ -1" ), "(2 ^ (-1))" );
    EXPECT_EQ( parsed( "2 * 3 ^ 2" ), "(2 * (3 ^ 2))" );
    EXPECT_EQ( parsed( "(2 ^ 3) ^ 2" ), "((2 ^ 3) ^ 2)" );
}

// Accept: A-08
TEST( ExprParser, ParsesUnitsReferencesAndCalls_WhenWrittenAsInTheSpec )
{
    EXPECT_EQ( parsed( "10 mm + 0.5 in" ), "(10 mm + 0.5 in)" );
    EXPECT_EQ( parsed( "0.5in" ), "0.5 in" );
    EXPECT_EQ( parsed( "90deg - 1 rad" ), "(90 deg - 1 rad)" );
    EXPECT_EQ( parsed( "25 \xC2\xB5m + 3 \xCE\xBCm + 1 um" ), "((25 \xC2\xB5m + 3 \xC2\xB5m) + 1 \xC2\xB5m)" );
    EXPECT_EQ( parsed( "1.5e3 mm" ), "1500 mm" );
    EXPECT_EQ( parsed( ".5 m" ), "0.5 m" );
    EXPECT_EQ( parsed( "Board.Width / 2" ), "(Board.Width / 2)" );
    EXPECT_EQ( parsed( "max(a, b ^ 2)" ), "max(a, (b ^ 2))" );
    EXPECT_EQ( parsed( "if(Board.Width > 50 mm, 2 mm, 1 mm)" ), "if((Board.Width > 50 mm), 2 mm, 1 mm)" );
    EXPECT_EQ( parsed( "atan2(y, x)" ), "atan2(y, x)" );
    EXPECT_EQ( parsed( "pi()" ), "pi()" );
    EXPECT_EQ( parsed( "  _a1 \t\r\n" ), "_a1" );
}

TEST( ExprParser, RecordsSourceRanges_WhenParsing )
{
    const app::Result<app::core::expr::NodePtr> root = app::core::expr::parseExpression( "  Board.Width + 10 mm" );

    ASSERT_TRUE( root.has_value() );

    const app::core::expr::Node& node   = **root;
    const auto*                  binary = std::get_if<Binary>( &node.value );

    ASSERT_NE( binary, nullptr );
    EXPECT_EQ( node.range, ( SourceRange{ 2, 19 } ) );
    EXPECT_EQ( binary->left->range, ( SourceRange{ 2, 11 } ) );
    EXPECT_EQ( binary->right->range, ( SourceRange{ 16, 5 } ) );
}

TEST( ExprParser, ReportsColumn_WhenSyntaxIsInvalid )
{
    const std::vector<std::pair<std::string_view, std::string_view>> cases = {
        { "1 + ", "Unexpected end of expression at column 5." },
        { "(1 + 2", "Expected ')' at column 7." },
        { "10 mn", "Unknown unit 'mn' at column 4." },
        { "2 # 3", "Unexpected character '#' at column 3." },
        { "1 < 2 < 3", "Comparisons cannot be chained at column 7." },
        { "Board. + 1", "Expected a name after '.' at column 8." },
        { "a.b(1)", "Function name 'a.b' cannot be dotted at column 4." },
        { "max(1 2)", "Expected ',' or ')' at column 7." },
        { "10 mm 5", "Unexpected '5' at column 7." },
        { "25 \xC2\xB5m + * 2", "Unexpected '*' at column 9." },
        { "1.", "Unexpected '.' at column 2." },
        { ")", "Unexpected ')' at column 1." },
        { "f(", "Unexpected end of expression at column 3." },
        { "1e999", "Number '1e999' is out of range at column 1." },
        { "\x01", "Unexpected control character U+0001 at column 1." },
        { "a \xFF", "Unexpected byte 0xFF at column 3." },
        { "\xE2\x82\xAC 5", "Unexpected character '\xE2\x82\xAC' at column 1." },
        { "", "Expression is empty." },
        { "  ", "Expression is empty." },
    };

    for( const auto& [text, message] : cases )
    {
        const app::Result<app::core::expr::NodePtr> root = app::core::expr::parseExpression( text );

        ASSERT_FALSE( root.has_value() ) << text;
        EXPECT_EQ( root.error().code, app::ErrorCode::InvalidArgument ) << text;
        EXPECT_EQ( root.error().message, message ) << text;
    }
}

TEST( ExprParser, ShowsCaretUnderColumn_WhenReportingAnError )
{
    const app::Result<app::core::expr::NodePtr> root = app::core::expr::parseExpression( "(1 + 2" );

    ASSERT_FALSE( root.has_value() );
    EXPECT_EQ( root.error().detail, "(1 + 2\n      ^" );
}

TEST( ExprParser, RejectsInput_WhenNestedDeeperThanTheLimit )
{
    EXPECT_EQ( parsed( repeated( "(", 60 ) + "1" + repeated( ")", 60 ) ), "1" );
    EXPECT_EQ( parsed( repeated( "-", 3 ) + "1" ), "(-(-(-1)))" );

    const std::string limit = "error: Expression is nested more than 64 levels deep";

    EXPECT_TRUE( parsed( repeated( "(", 70 ) + "1" + repeated( ")", 70 ) ).starts_with( limit ) );
    EXPECT_TRUE( parsed( repeated( "-", 100 ) + "1" ).starts_with( limit ) );
    EXPECT_TRUE( parsed( "2" + repeated( "^2", 100 ) ).starts_with( limit ) );
    EXPECT_TRUE( parsed( repeated( "f(", 100 ) + repeated( ")", 100 ) ).starts_with( limit ) );
    EXPECT_TRUE( parsed( repeated( "(", 100'000 ) ).starts_with( limit ) );
}

TEST( ExprParser, SplitsTokensGreedily_WhenOperatorsAndExponentsTouch )
{
    const app::Result<std::vector<app::core::expr::Token>> tokens = app::core::expr::tokenize( "a<=b>2em-1e+3" );

    ASSERT_TRUE( tokens.has_value() );

    std::vector<TokenKind>   kinds;
    std::vector<std::string> texts;

    kinds.reserve( tokens->size() );
    texts.reserve( tokens->size() );

    for( const app::core::expr::Token& token : *tokens )
    {
        kinds.push_back( token.kind );
        texts.push_back( token.text );
    }

    const std::vector<TokenKind>   expectedKinds = { TokenKind::Identifier,
                                                     TokenKind::LessEqual,
                                                     TokenKind::Identifier,
                                                     TokenKind::Greater,
                                                     TokenKind::Number,
                                                     TokenKind::Identifier,
                                                     TokenKind::Minus,
                                                     TokenKind::Number,
                                                     TokenKind::End };
    const std::vector<std::string> expectedTexts = { "a", "<=", "b", ">", "2", "em", "-", "1e+3", "" };

    EXPECT_EQ( kinds, expectedKinds );
    EXPECT_EQ( texts, expectedTexts );
    EXPECT_EQ( tokens->at( 7 ).number, 1000.0 );
    EXPECT_EQ( app::core::expr::columnOf( "\xC2\xB5m x", 4 ), 4U );
}
