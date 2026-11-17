// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/IParameterScope.h
/// @brief  Named parameter lookup through nested scopes (project, assembly, part, ...), used by the expression
///         evaluator.
/// @req    A-08

#pragma once

#include "core/expr/Evaluator.h"
#include "core/Quantity.h"
#include "core/Result.h"

#include <optional>
#include <string_view>

namespace app::core::expr
{

/// @brief One level of parameter scopes. Scopes form a chain from the innermost (for example a part) to the outermost
///        (the project); a name resolves in the nearest scope that defines it.
///
/// A dotted name such as `Board.Width` is one parameter name, looked up whole in each scope.
class IParameterScope
{
public:
    IParameterScope()                                    = default;
    IParameterScope( const IParameterScope& )            = delete;
    IParameterScope& operator=( const IParameterScope& ) = delete;
    IParameterScope( IParameterScope&& )                 = delete;
    IParameterScope& operator=( IParameterScope&& )      = delete;
    virtual ~IParameterScope()                           = default;

    /// @brief  Name of the scope for messages, for example "project" or a part name.
    /// @return The name.
    [[nodiscard]] virtual std::string_view scopeName() const noexcept = 0;

    /// @brief  The enclosing scope.
    /// @return The parent, or null for the outermost scope.
    [[nodiscard]] virtual const IParameterScope* parent() const noexcept = 0;

    /// @brief  Value of a parameter defined in this scope itself (parents are not searched).
    /// @param  name  Dotted parameter name.
    /// @return Nothing when this scope does not define @p name; otherwise its value or the error evaluating it
    ///         (ErrorCode::Conflict for a cycle, ErrorCode::InvalidArgument for an invalid expression).
    [[nodiscard]] virtual std::optional<Result<Quantity>> findLocal( std::string_view name ) const = 0;
};

/// @brief  Resolve a name in a scope and then in its parents.
/// @param  scope  Innermost scope.
/// @param  name   Dotted parameter name.
/// @return The value from the nearest scope that defines @p name, its evaluation error, or ErrorCode::NotFound
///         "Unknown parameter 'name'." when no scope in the chain defines it.
[[nodiscard]] Result<Quantity> lookup( const IParameterScope& scope, std::string_view name );

/// @brief  A ReferenceResolver that calls lookup() on a scope. The scope must outlive the resolver.
/// @param  scope  Innermost scope for the expression being evaluated.
/// @return The resolver.
[[nodiscard]] ReferenceResolver resolverFor( const IParameterScope& scope );

} // namespace app::core::expr
