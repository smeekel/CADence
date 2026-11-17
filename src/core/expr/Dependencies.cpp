// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Dependencies.cpp
/// @brief  Collection of the parameter references of an expression.

#include "core/expr/Dependencies.h"

#include <algorithm>
#include <type_traits>
#include <utility>
#include <variant>

namespace app::core::expr
{

namespace
{

void collect( const Node& node, std::vector<std::string>& names )
{
    std::visit(
        [&]( const auto& value )
        {
            using T = std::decay_t<decltype( value )>;

            if constexpr( std::is_same_v<T, Reference> )
            {
                std::string name = value.dottedName();

                if( std::ranges::find( names, name ) == names.end() )
                {
                    names.push_back( std::move( name ) );
                }
            }
            else if constexpr( std::is_same_v<T, Unary> )
            {
                collect( *value.operand, names );
            }
            else if constexpr( std::is_same_v<T, Binary> )
            {
                collect( *value.left, names );
                collect( *value.right, names );
            }
            else if constexpr( std::is_same_v<T, Call> )
            {
                for( const NodePtr& argument : value.arguments )
                {
                    collect( *argument, names );
                }
            }
        },
        node.value );
}

} // namespace

std::vector<std::string> referencedNames( const Node& root )
{
    std::vector<std::string> names;

    collect( root, names );
    return names;
}

} // namespace app::core::expr
