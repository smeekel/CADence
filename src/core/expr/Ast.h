// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Ast.h
/// @brief  Syntax tree of the shared expression language (grammar: docs/format/expressions.md).
/// @req    A-08

#pragma once

#include "core/Units.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace app::core::expr
{

/// @brief Byte range of a node or token in the source text.
struct SourceRange
{
    std::size_t offset = 0; ///< First byte.
    std::size_t length = 0; ///< Number of bytes.

    /// @brief Ranges are equal when offset and length are equal.
    [[nodiscard]] friend constexpr bool operator==( const SourceRange&, const SourceRange& ) noexcept = default;
};

/// @brief Prefix operators.
enum class UnaryOp : std::uint8_t
{
    Plus,  ///< `+a`
    Minus, ///< `-a`
};

/// @brief Infix operators.
enum class BinaryOp : std::uint8_t
{
    Add,          ///< `a + b`
    Subtract,     ///< `a - b`
    Multiply,     ///< `a * b`
    Divide,       ///< `a / b`
    Power,        ///< `a ^ b` (right-associative)
    Less,         ///< `a < b`
    LessEqual,    ///< `a <= b`
    Greater,      ///< `a > b`
    GreaterEqual, ///< `a >= b`
    Equal,        ///< `a == b`
    NotEqual,     ///< `a != b`
};

/// @brief  Source spelling of an operator.
/// @param  op  Operator.
/// @return For example "-".
[[nodiscard]] std::string_view symbol( UnaryOp op ) noexcept;

/// @brief  Source spelling of an operator.
/// @param  op  Operator.
/// @return For example "<=".
[[nodiscard]] std::string_view symbol( BinaryOp op ) noexcept;

struct Node;

/// @brief Owning pointer to an immutable node.
using NodePtr = std::unique_ptr<const Node>;

/// @brief A number with an optional unit: `10`, `0.5 in`, `2.5e-3 m`.
struct NumberLiteral
{
    double              value = 0.0; ///< The number as written, in @ref unit.
    std::optional<Unit> unit;        ///< The unit written after the number, if any.
};

/// @brief A reference to a named parameter, possibly dotted: `Width`, `Board.Width`.
struct Reference
{
    std::vector<std::string> path; ///< Name segments, at least one.

    /// @brief  The segments joined with '.'.
    /// @return For example "Board.Width".
    [[nodiscard]] std::string dottedName() const;
};

/// @brief A prefix operation.
struct Unary
{
    UnaryOp op = UnaryOp::Plus; ///< Operator.
    NodePtr operand;            ///< Operand, never null.
};

/// @brief An infix operation.
struct Binary
{
    BinaryOp    op = BinaryOp::Add; ///< Operator.
    NodePtr     left;               ///< Left operand, never null.
    NodePtr     right;              ///< Right operand, never null.
    std::size_t operatorOffset = 0; ///< Byte offset of the operator, where evaluation errors point.
};

/// @brief A function call: `max(a, b)`. The parser accepts any name; the evaluator checks it (S0.03.7).
struct Call
{
    std::string          name;      ///< Function name (not dotted).
    std::vector<NodePtr> arguments; ///< Arguments in order, none null.
};

/// @brief A node of the syntax tree with the source range it was parsed from.
struct Node
{
    std::variant<NumberLiteral, Reference, Unary, Binary, Call> value; ///< The node kind and its data.
    SourceRange                                                 range; ///< Where it came from.
};

/// @brief  Fully parenthesised text of a tree, for tests and diagnostics: `(1 + (2 * 3))`, `max(a, (b ^ 2))`.
/// @param  node  Root node.
/// @return The text. Numbers use the shortest round-trip form; units follow with a space (`10 mm`).
[[nodiscard]] std::string toText( const Node& node );

} // namespace app::core::expr
