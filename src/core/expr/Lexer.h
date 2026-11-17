// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Lexer.h
/// @brief  Tokeniser of the shared expression language (grammar: docs/format/expressions.md).
/// @req    A-08

#pragma once

#include "core/expr/Ast.h"
#include "core/Result.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace app::core::expr
{

/// @brief Token kinds.
enum class TokenKind : std::uint8_t
{
    Number,       ///< `12`, `0.5`, `.5`, `1e-3`
    Identifier,   ///< A name segment or unit: `Board`, `mm`, `µm`
    Plus,         ///< `+`
    Minus,        ///< `-`
    Star,         ///< `*`
    Slash,        ///< `/`
    Caret,        ///< `^`
    LeftParen,    ///< `(`
    RightParen,   ///< `)`
    Comma,        ///< `,`
    Dot,          ///< `.` between name segments
    Less,         ///< `<`
    LessEqual,    ///< `<=`
    Greater,      ///< `>`
    GreaterEqual, ///< `>=`
    EqualEqual,   ///< `==`
    NotEqual,     ///< `!=`
    End,          ///< End of the text; always the last token.
};

/// @brief One token of an expression.
struct Token
{
    TokenKind   kind = TokenKind::End; ///< Kind.
    std::string text;                  ///< The characters as written (empty for End).
    SourceRange range;                 ///< Where it is in the source.
    double      number = 0.0;          ///< Value of a Number token.
};

/// @brief  1-based column of a byte offset, counting Unicode code points (UTF-8), as shown to the user.
/// @param  source  Source text.
/// @param  offset  Byte offset; clamped to the text length.
/// @return The column.
[[nodiscard]] std::size_t columnOf( std::string_view source, std::size_t offset ) noexcept;

/// @brief  Syntax error at a position: message "<what> at column <n>.", detail the source with a caret under it.
/// @param  source  Source text.
/// @param  offset  Byte offset of the problem.
/// @param  what    Description without the position, for example "Expected ')'".
/// @return ErrorCode::InvalidArgument.
[[nodiscard]] std::unexpected<Error> syntaxError( std::string_view source, std::size_t offset, std::string_view what );

/// @brief  Split an expression into tokens.
/// @param  source  UTF-8 text.
/// @return The tokens, ending with one End token, or ErrorCode::InvalidArgument naming the column of an unexpected
///         character or a malformed number ("Unexpected character '#' at column 4.").
[[nodiscard]] Result<std::vector<Token>> tokenize( std::string_view source );

} // namespace app::core::expr
