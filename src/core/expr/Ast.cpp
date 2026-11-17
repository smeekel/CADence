// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Ast.cpp
/// @brief  Operator spellings and the parenthesised text form of expression trees.

#include "core/expr/Ast.h"

#include <type_traits>

namespace app::core::expr
{

std::string_view symbol( UnaryOp op ) noexcept
{
    return op == UnaryOp::Minus ? "-" : "+";
}

std::string_view symbol( BinaryOp op ) noexcept
{
    switch( op )
    {
        case BinaryOp::Add:
            return "+";
        case BinaryOp::Subtract:
            return "-";
        case BinaryOp::Multiply:
            return "*";
        case BinaryOp::Divide:
            return "/";
        case BinaryOp::Power:
            return "^";
        case BinaryOp::Less:
            return "<";
        case BinaryOp::LessEqual:
            return "<=";
        case BinaryOp::Greater:
            return ">";
        case BinaryOp::GreaterEqual:
            return ">=";
        case BinaryOp::Equal:
            return "==";
        case BinaryOp::NotEqual:
            return "!=";
    }

    return "?";
}

std::string Reference::dottedName() const
{
    std::string name;

    for( const std::string& segment : path )
    {
        if( !name.empty() )
        {
            name += '.';
        }

        name += segment;
    }

    return name;
}

std::string toText( const Node& node )
{
    return std::visit(
        []( const auto& value ) -> std::string
        {
            using T = std::decay_t<decltype( value )>;

            if constexpr( std::is_same_v<T, NumberLiteral> )
            {
                std::string text = formatNumber( value.value, std::nullopt );

                if( value.unit.has_value() )
                {
                    text += ' ';
                    text += app::core::symbol( *value.unit );
                }

                return text;
            }
            else if constexpr( std::is_same_v<T, Reference> )
            {
                return value.dottedName();
            }
            else if constexpr( std::is_same_v<T, Unary> )
            {
                return "(" + std::string( symbol( value.op ) ) + toText( *value.operand ) + ")";
            }
            else if constexpr( std::is_same_v<T, Binary> )
            {
                return "(" + toText( *value.left ) + " " + std::string( symbol( value.op ) ) + " "
                       + toText( *value.right ) + ")";
            }
            else
            {
                std::string text = value.name + "(";

                for( std::size_t i = 0; i < value.arguments.size(); ++i )
                {
                    if( i > 0 )
                    {
                        text += ", ";
                    }

                    text += toText( *value.arguments[i] );
                }

                return text + ")";
            }
        },
        node.value );
}

} // namespace app::core::expr
