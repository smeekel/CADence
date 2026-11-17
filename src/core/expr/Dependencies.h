// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Dependencies.h
/// @brief  Parameter references of an expression, for dependency tracking (A-07) and "where used" lists.
/// @req    A-08, A-07

#pragma once

#include "core/expr/Ast.h"

#include <string>
#include <vector>

namespace app::core::expr
{

/// @brief  Dotted names of the parameters an expression refers to, each once, in order of first appearance.
///
/// Function names are not references and are not listed. Both branches of `if` are listed, because a change to either
/// can change the value when the condition changes.
///
/// @param  root  Syntax tree.
/// @return The names, for example { "Board.Width", "t" } for `Board.Width - 2 * t + t`.
[[nodiscard]] std::vector<std::string> referencedNames( const Node& root );

} // namespace app::core::expr
