// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Evaluator.h
/// @brief  Evaluation of expression trees with unit checking (rules: docs/format/expressions.md).
/// @req    A-08

#pragma once

#include "core/expr/Ast.h"
#include "core/Quantity.h"
#include "core/Result.h"

#include <functional>
#include <string_view>

namespace app::core::expr
{

/// @brief Resolves a parameter reference by its dotted name (`Board.Width`) to a value in internal units, or an error
///        (for example ErrorCode::NotFound) whose message the evaluator reports at the reference's column.
using ReferenceResolver = std::function<Result<Quantity>( std::string_view dottedName )>;

/// @brief  Evaluate a syntax tree.
///
/// Numbers with units become internal units (mm, rad). `+ -` and comparisons need equal dimensions (`1 mm + 1 deg` is
/// an error), `* /` combine them, an exponent must be dimensionless and must leave whole powers of each dimension.
/// Comparisons give 1 or 0 and use the confusion tolerance (1e-7 in internal units). `if` evaluates only the branch
/// it selects.
///
/// @param  root     Tree from parseExpression().
/// @param  source   The text the tree was parsed from, for error columns.
/// @param  resolve  Parameter lookup; without one, every reference is an error.
/// @return The value, or an error whose message names the 1-based column of the operator, function or reference at
///         fault ("Cannot add length and angle at column 6.").
[[nodiscard]] Result<Quantity>
evaluate( const Node& root, std::string_view source, const ReferenceResolver& resolve = {} );

/// @brief  Parse and evaluate an expression.
/// @param  source   UTF-8 text.
/// @param  resolve  Parameter lookup; without one, every reference is an error.
/// @return The value, or the syntax or evaluation error.
[[nodiscard]] Result<Quantity> evaluateExpression( std::string_view source, const ReferenceResolver& resolve = {} );

} // namespace app::core::expr
