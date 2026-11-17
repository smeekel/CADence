// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/ParameterScope.h
/// @brief  A parameter scope that stores an expression per name and evaluates it on lookup, rejecting cycles.
/// @req    A-08, A-07

#pragma once

#include "core/expr/Ast.h"
#include "core/expr/IParameterScope.h"
#include "core/Result.h"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace app::core::expr
{

/// @brief Parameters of one scope, each defined by an expression that may refer to parameters of this scope and of
///        its parents.
///
/// Values are computed on every lookup, so they always reflect the current expressions. A reference inside a
/// parameter's expression resolves from this scope outwards, so an inner `t = t * 2` refers to itself and is a cycle.
/// Not thread-safe: cycle detection keeps the chain being evaluated, so look up from one thread at a time.
class ParameterScope final : public IParameterScope
{
public:
    /// @brief Empty scope.
    /// @param name    Scope name for messages.
    /// @param parent  Enclosing scope, or null; it must outlive this scope.
    explicit ParameterScope( std::string name, const IParameterScope* parent = nullptr );

    /// @brief  Define or replace a parameter.
    /// @param  name        Dotted name: names as in the expression grammar, joined by '.' (`Board.Width`).
    /// @param  expression  Expression text; parsed now, evaluated on lookup.
    /// @return Nothing, ErrorCode::InvalidArgument "Invalid parameter name 'n'.", or the syntax error of the
    /// expression.
    [[nodiscard]] Result<> define( std::string_view name, std::string_view expression );

    /// @brief  Remove a parameter.
    /// @param  name  Parameter name.
    /// @return True if it was defined.
    bool remove( std::string_view name );

    /// @brief  Expression text of a parameter.
    /// @param  name  Parameter name.
    /// @return The text, or nothing when this scope does not define @p name.
    [[nodiscard]] std::optional<std::string> expressionOf( std::string_view name ) const;

    /// @brief  Names defined in this scope.
    /// @return The names in ascending byte order.
    [[nodiscard]] std::vector<std::string> names() const;

    /// @brief  Parameters referred to by a parameter's expression (direct dependencies only).
    /// @param  name  Parameter name.
    /// @return The names in order of first appearance, or ErrorCode::NotFound.
    [[nodiscard]] Result<std::vector<std::string>> dependenciesOf( std::string_view name ) const;

    [[nodiscard]] std::string_view scopeName() const noexcept override;

    [[nodiscard]] const IParameterScope* parent() const noexcept override;

    [[nodiscard]] std::optional<Result<Quantity>> findLocal( std::string_view name ) const override;

private:
    struct Entry
    {
        std::string expression;
        NodePtr     tree;
    };

    std::string                               m_name;
    const IParameterScope*                    m_parent = nullptr;
    std::map<std::string, Entry, std::less<>> m_entries;
    mutable std::vector<std::string>          m_evaluating;
};

} // namespace app::core::expr
