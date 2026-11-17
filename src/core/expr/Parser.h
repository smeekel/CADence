// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Parser.h
/// @brief  Parser of the shared expression language (grammar: docs/format/expressions.md).
/// @req    A-08

#pragma once

#include "core/expr/Ast.h"
#include "core/Result.h"

#include <cstddef>
#include <string_view>

namespace app::core::expr
{

/// @brief Deepest nesting the parser accepts, so that hostile input cannot exhaust the stack. The whole expression,
///        each parenthesis or call argument list, each prefix sign and each `^` adds a level.
inline constexpr std::size_t kMaxNestingDepth = 64;

/// @brief  Parse an expression into a syntax tree.
///
/// Precedence from loosest to tightest: comparison (`< <= > >= == !=`, not chainable), `+ -`, `* /`, prefix `+ -`,
/// `^` (right-associative, so `2 ^ 3 ^ 2` is `2 ^ 9` and `-2 ^ 2` is `-(2 ^ 2)`). A unit may follow a number literal
/// only (`10 mm`, `0.5in`). Names are references, possibly dotted (`Board.Width`); a name followed by `(` is a call.
///
/// @param  source  UTF-8 text.
/// @return The root node, or ErrorCode::InvalidArgument whose message names the 1-based column of the problem
///         ("Expected ')' at column 8.") and whose detail shows the source with a caret under that column.
[[nodiscard]] Result<NodePtr> parseExpression( std::string_view source );

} // namespace app::core::expr
