// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Parser.cpp
/// @brief  Recursive-descent parser of the shared expression language.

#include "core/expr/Parser.h"

#include "core/expr/Lexer.h"

#include <format>
#include <optional>
#include <utility>
#include <vector>

namespace app::core::expr
{

namespace
{

template <typename T>
[[nodiscard]] NodePtr makeNode( T value, SourceRange range )
{
    return std::make_unique<const Node>( Node{ std::move( value ), range } );
}

[[nodiscard]] SourceRange spanning( std::size_t begin, const Node& last ) noexcept
{
    return { begin, last.range.offset + last.range.length - begin };
}

[[nodiscard]] std::optional<BinaryOp> comparisonOp( TokenKind kind ) noexcept
{
    switch( kind )
    {
        case TokenKind::Less:
            return BinaryOp::Less;
        case TokenKind::LessEqual:
            return BinaryOp::LessEqual;
        case TokenKind::Greater:
            return BinaryOp::Greater;
        case TokenKind::GreaterEqual:
            return BinaryOp::GreaterEqual;
        case TokenKind::EqualEqual:
            return BinaryOp::Equal;
        case TokenKind::NotEqual:
            return BinaryOp::NotEqual;
        default:
            return std::nullopt;
    }
}

class Parser
{
public:
    Parser( std::string_view source, std::vector<Token> tokens )
        : m_source( source )
        , m_tokens( std::move( tokens ) )
    {
    }

    [[nodiscard]] Result<NodePtr> parseAll()
    {
        Result<NodePtr> root = comparison();

        if( root.has_value() && peek().kind != TokenKind::End )
        {
            return unexpected( peek() );
        }

        return root;
    }

private:
    // Counts nesting for kMaxNestingDepth while a rule that can recurse is active.
    class DepthGuard
    {
    public:
        explicit DepthGuard( std::size_t& depth ) noexcept
            : m_depth( depth )
        {
            ++m_depth;
        }

        DepthGuard( const DepthGuard& )            = delete;
        DepthGuard& operator=( const DepthGuard& ) = delete;

        ~DepthGuard()
        {
            --m_depth;
        }

    private:
        std::size_t& m_depth;
    };

    [[nodiscard]] const Token& peek() const noexcept
    {
        return m_tokens[m_position];
    }

    const Token& advance() noexcept
    {
        const Token& token = m_tokens[m_position];

        if( token.kind != TokenKind::End )
        {
            ++m_position;
        }

        return token;
    }

    [[nodiscard]] std::unexpected<Error> unexpected( const Token& token ) const
    {
        if( token.kind == TokenKind::End )
        {
            return syntaxError( m_source, token.range.offset, "Unexpected end of expression" );
        }

        return syntaxError( m_source, token.range.offset, std::format( "Unexpected '{}'", token.text ) );
    }

    [[nodiscard]] std::optional<std::unexpected<Error>> tooDeep() const
    {
        if( m_depth > kMaxNestingDepth )
        {
            return syntaxError( m_source,
                                peek().range.offset,
                                std::format( "Expression is nested more than {} levels deep", kMaxNestingDepth ) );
        }

        return std::nullopt;
    }

    // comparison := additive [ ( "<" | "<=" | ">" | ">=" | "==" | "!=" ) additive ]
    [[nodiscard]] Result<NodePtr> comparison()
    {
        const DepthGuard guard( m_depth );

        if( auto error = tooDeep() )
        {
            return *error;
        }

        Result<NodePtr> left = additive();

        if( !left.has_value() )
        {
            return left;
        }

        const std::optional<BinaryOp> op = comparisonOp( peek().kind );

        if( !op.has_value() )
        {
            return left;
        }

        advance();

        Result<NodePtr> right = additive();

        if( !right.has_value() )
        {
            return right;
        }

        if( comparisonOp( peek().kind ).has_value() )
        {
            return syntaxError( m_source, peek().range.offset, "Comparisons cannot be chained" );
        }

        const SourceRange range = spanning( ( *left )->range.offset, **right );

        return makeNode( Binary{ *op, std::move( *left ), std::move( *right ) }, range );
    }

    // additive := term { ( "+" | "-" ) term }
    [[nodiscard]] Result<NodePtr> additive()
    {
        Result<NodePtr> left = term();

        while( left.has_value() && ( peek().kind == TokenKind::Plus || peek().kind == TokenKind::Minus ) )
        {
            const BinaryOp op = advance().kind == TokenKind::Plus ? BinaryOp::Add : BinaryOp::Subtract;

            Result<NodePtr> right = term();

            if( !right.has_value() )
            {
                return right;
            }

            const SourceRange range = spanning( ( *left )->range.offset, **right );

            left = makeNode( Binary{ op, std::move( *left ), std::move( *right ) }, range );
        }

        return left;
    }

    // term := unary { ( "*" | "/" ) unary }
    [[nodiscard]] Result<NodePtr> term()
    {
        Result<NodePtr> left = unary();

        while( left.has_value() && ( peek().kind == TokenKind::Star || peek().kind == TokenKind::Slash ) )
        {
            const BinaryOp op = advance().kind == TokenKind::Star ? BinaryOp::Multiply : BinaryOp::Divide;

            Result<NodePtr> right = unary();

            if( !right.has_value() )
            {
                return right;
            }

            const SourceRange range = spanning( ( *left )->range.offset, **right );

            left = makeNode( Binary{ op, std::move( *left ), std::move( *right ) }, range );
        }

        return left;
    }

    // unary := ( "+" | "-" ) unary | power
    [[nodiscard]] Result<NodePtr> unary()
    {
        if( peek().kind != TokenKind::Plus && peek().kind != TokenKind::Minus )
        {
            return power();
        }

        const DepthGuard guard( m_depth );

        if( auto error = tooDeep() )
        {
            return *error;
        }

        const Token&    sign    = advance();
        const UnaryOp   op      = sign.kind == TokenKind::Plus ? UnaryOp::Plus : UnaryOp::Minus;
        Result<NodePtr> operand = unary();

        if( !operand.has_value() )
        {
            return operand;
        }

        const SourceRange range = spanning( sign.range.offset, **operand );

        return makeNode( Unary{ op, std::move( *operand ) }, range );
    }

    // power := primary [ "^" unary ]   (the exponent recurses through unary, so "^" is right-associative)
    [[nodiscard]] Result<NodePtr> power()
    {
        Result<NodePtr> base = primary();

        if( !base.has_value() || peek().kind != TokenKind::Caret )
        {
            return base;
        }

        advance();

        const DepthGuard guard( m_depth );

        if( auto error = tooDeep() )
        {
            return *error;
        }

        Result<NodePtr> exponent = unary();

        if( !exponent.has_value() )
        {
            return exponent;
        }

        const SourceRange range = spanning( ( *base )->range.offset, **exponent );

        return makeNode( Binary{ BinaryOp::Power, std::move( *base ), std::move( *exponent ) }, range );
    }

    // primary := number [ unit ] | name { "." name } | name "(" [ comparison { "," comparison } ] ")"
    //          | "(" comparison ")"
    [[nodiscard]] Result<NodePtr> primary()
    {
        const Token& first = peek();

        switch( first.kind )
        {
            case TokenKind::Number:
                return numberLiteral();
            case TokenKind::Identifier:
                return referenceOrCall();
            case TokenKind::LeftParen:
            {
                advance();

                Result<NodePtr> inner = comparison();

                if( !inner.has_value() )
                {
                    return inner;
                }

                if( peek().kind != TokenKind::RightParen )
                {
                    return syntaxError( m_source, peek().range.offset, "Expected ')'" );
                }

                advance();
                return inner;
            }
            default:
                return unexpected( first );
        }
    }

    [[nodiscard]] Result<NodePtr> numberLiteral()
    {
        const Token&  number = advance();
        NumberLiteral literal{ number.number, std::nullopt };
        SourceRange   range = number.range;

        if( peek().kind == TokenKind::Identifier )
        {
            const Token&              unitToken = peek();
            const std::optional<Unit> unit      = parseUnit( unitToken.text );

            if( !unit.has_value() )
            {
                return syntaxError( m_source,
                                    unitToken.range.offset,
                                    std::format( "Unknown unit '{}'", unitToken.text ) );
            }

            advance();
            literal.unit = unit;
            range.length = unitToken.range.offset + unitToken.range.length - range.offset;
        }

        return makeNode( literal, range );
    }

    [[nodiscard]] Result<NodePtr> referenceOrCall()
    {
        const Token& name = advance();
        Reference    reference{ { name.text } };
        std::size_t  end = name.range.offset + name.range.length;

        if( peek().kind == TokenKind::LeftParen )
        {
            return call( name );
        }

        while( peek().kind == TokenKind::Dot )
        {
            advance();

            if( peek().kind != TokenKind::Identifier )
            {
                return syntaxError( m_source, peek().range.offset, "Expected a name after '.'" );
            }

            const Token& segment = advance();

            reference.path.push_back( segment.text );
            end = segment.range.offset + segment.range.length;
        }

        if( peek().kind == TokenKind::LeftParen )
        {
            return syntaxError( m_source,
                                peek().range.offset,
                                std::format( "Function name '{}' cannot be dotted", reference.dottedName() ) );
        }

        return makeNode( std::move( reference ), SourceRange{ name.range.offset, end - name.range.offset } );
    }

    [[nodiscard]] Result<NodePtr> call( const Token& name )
    {
        advance();

        Call node{ name.text, {} };

        if( peek().kind != TokenKind::RightParen )
        {
            while( true )
            {
                Result<NodePtr> argument = comparison();

                if( !argument.has_value() )
                {
                    return argument;
                }

                node.arguments.push_back( std::move( *argument ) );

                if( peek().kind == TokenKind::Comma )
                {
                    advance();
                    continue;
                }

                if( peek().kind != TokenKind::RightParen )
                {
                    return syntaxError( m_source, peek().range.offset, "Expected ',' or ')'" );
                }

                break;
            }
        }

        const Token& close = advance();

        return makeNode(
            std::move( node ),
            SourceRange{ name.range.offset, close.range.offset + close.range.length - name.range.offset } );
    }

    std::string_view   m_source;
    std::vector<Token> m_tokens;
    std::size_t        m_position = 0;
    std::size_t        m_depth    = 0;
};

} // namespace

Result<NodePtr> parseExpression( std::string_view source )
{
    Result<std::vector<Token>> tokens = tokenize( source );

    if( !tokens.has_value() )
    {
        return std::unexpected( std::move( tokens.error() ) );
    }

    if( tokens->size() == 1 )
    {
        return fail( ErrorCode::InvalidArgument, "Expression is empty." );
    }

    return Parser( source, std::move( *tokens ) ).parseAll();
}

} // namespace app::core::expr
