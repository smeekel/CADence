// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/ParameterScope.cpp
/// @brief  Scope chain lookup and the expression-backed parameter scope.

#include "core/expr/ParameterScope.h"

#include "core/expr/Dependencies.h"
#include "core/expr/Lexer.h"
#include "core/expr/Parser.h"

#include <algorithm>
#include <format>
#include <utility>

namespace app::core::expr
{

namespace
{

// A parameter name is Identifier { Dot Identifier } with nothing else, not even spaces.
[[nodiscard]] bool isValidName( std::string_view name )
{
    const Result<std::vector<Token>> tokens = tokenize( name );

    if( !tokens.has_value() || tokens->size() < 2 || tokens->size() % 2 != 0 )
    {
        return false;
    }

    for( std::size_t i = 0; i + 1 < tokens->size(); ++i )
    {
        const TokenKind expected = i % 2 == 0 ? TokenKind::Identifier : TokenKind::Dot;

        if( ( *tokens )[i].kind != expected )
        {
            return false;
        }
    }

    // Spaces between segments would tokenize too; the name must be written without them.
    std::size_t length = 0;

    for( const Token& token : *tokens )
    {
        length += token.text.size();
    }

    return length == name.size();
}

} // namespace

Result<Quantity> lookup( const IParameterScope& scope, std::string_view name )
{
    for( const IParameterScope* current = &scope; current != nullptr; current = current->parent() )
    {
        std::optional<Result<Quantity>> found = current->findLocal( name );

        if( found.has_value() )
        {
            return std::move( *found );
        }
    }

    return fail( ErrorCode::NotFound, std::format( "Unknown parameter '{}'.", name ) );
}

ReferenceResolver resolverFor( const IParameterScope& scope )
{
    return [&scope]( std::string_view name )
    {
        return lookup( scope, name );
    };
}

ParameterScope::ParameterScope( std::string name, const IParameterScope* parent )
    : m_name( std::move( name ) )
    , m_parent( parent )
{
}

Result<> ParameterScope::define( std::string_view name, std::string_view expression )
{
    if( !isValidName( name ) )
    {
        return fail( ErrorCode::InvalidArgument, std::format( "Invalid parameter name '{}'.", name ) );
    }

    Result<NodePtr> tree = parseExpression( expression );

    if( !tree.has_value() )
    {
        return std::unexpected( std::move( tree.error() ) );
    }

    m_entries.insert_or_assign( std::string( name ), Entry{ std::string( expression ), std::move( *tree ) } );
    return ok();
}

bool ParameterScope::remove( std::string_view name )
{
    const auto found = m_entries.find( name );

    if( found == m_entries.end() )
    {
        return false;
    }

    m_entries.erase( found );
    return true;
}

std::optional<std::string> ParameterScope::expressionOf( std::string_view name ) const
{
    const auto found = m_entries.find( name );

    if( found == m_entries.end() )
    {
        return std::nullopt;
    }

    return found->second.expression;
}

std::vector<std::string> ParameterScope::names() const
{
    std::vector<std::string> result;

    result.reserve( m_entries.size() );

    for( const auto& [name, entry] : m_entries )
    {
        result.push_back( name );
    }

    return result;
}

Result<std::vector<std::string>> ParameterScope::dependenciesOf( std::string_view name ) const
{
    const auto found = m_entries.find( name );

    if( found == m_entries.end() )
    {
        return fail( ErrorCode::NotFound, std::format( "Unknown parameter '{}'.", name ) );
    }

    return referencedNames( *found->second.tree );
}

std::string_view ParameterScope::scopeName() const noexcept
{
    return m_name;
}

const IParameterScope* ParameterScope::parent() const noexcept
{
    return m_parent;
}

std::optional<Result<Quantity>> ParameterScope::findLocal( std::string_view name ) const
{
    const auto found = m_entries.find( name );

    if( found == m_entries.end() )
    {
        return std::nullopt;
    }

    if( std::ranges::find( m_evaluating, name ) != m_evaluating.end() )
    {
        std::string chain;

        for( auto it = std::ranges::find( m_evaluating, name ); it != m_evaluating.end(); ++it )
        {
            chain += *it + " -> ";
        }

        return fail( ErrorCode::Conflict, std::format( "Parameter cycle in {}: {}{}.", m_name, chain, name ) );
    }

    m_evaluating.emplace_back( name );

    struct PopOnExit
    {
        std::vector<std::string>& chain;

        ~PopOnExit()
        {
            chain.pop_back();
        }
    } const popOnExit{ m_evaluating };

    // Errors returned by lookups of other parameters already name their parameter (or the cycle); only errors in this
    // parameter's own expression are prefixed with its name.
    bool                    nestedError = false;
    const ReferenceResolver resolve     = [&]( std::string_view reference ) -> Result<Quantity>
    {
        Result<Quantity> value = lookup( *this, reference );

        if( !value.has_value() && value.error().code != ErrorCode::NotFound )
        {
            nestedError = true;
        }

        return value;
    };

    Result<Quantity> value = evaluate( *found->second.tree, found->second.expression, resolve );

    if( value.has_value() || nestedError )
    {
        return value;
    }

    return fail( ErrorCode::InvalidArgument,
                 std::format( "Parameter '{}': {}", name, value.error().message ),
                 value.error().detail );
}

} // namespace app::core::expr
