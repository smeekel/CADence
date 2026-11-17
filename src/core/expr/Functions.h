// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Functions.h
/// @brief  Built-in functions of the expression language and their unit rules (docs/format/expressions.md).
/// @req    A-08

#pragma once

#include "core/Quantity.h"

#include <cstddef>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace app::core::expr
{

/// @brief A value, or a message without position or final period; the evaluator adds the column.
using Evaluated = std::expected<Quantity, std::string>;

/// @brief  Whether a name is a built-in function: `sin cos tan asin acos atan atan2 sqrt abs min max floor ceil round
///         if`.
/// @param  name  Function name as written (case-sensitive).
/// @return True for a built-in.
[[nodiscard]] bool isFunction( std::string_view name ) noexcept;

/// @brief  Check the number of arguments of a built-in function.
/// @param  name   A built-in function name.
/// @param  count  Number of arguments written.
/// @return A message such as "atan2 takes 2 arguments, not 1", or nothing when the count is allowed.
[[nodiscard]] std::optional<std::string> checkArgumentCount( std::string_view name, std::size_t count );

/// @brief  Apply a built-in function other than `if` (which the evaluator handles so that only the chosen branch is
///         evaluated).
/// @param  name       A built-in function name whose argument count has been checked.
/// @param  arguments  Evaluated arguments.
/// @return The result, or a message naming the function and the problem ("sqrt of a negative number").
[[nodiscard]] Evaluated applyFunction( std::string_view name, std::span<const Quantity> arguments );

} // namespace app::core::expr
