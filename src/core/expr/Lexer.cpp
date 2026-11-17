// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Lexer.cpp
/// @brief  Tokeniser of the shared expression language.

#include "core/expr/Lexer.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <format>
#include <system_error>

namespace app::core::expr
{

namespace
{

constexpr std::string_view kMicroSign = "\xC2\xB5";
constexpr std::string_view kGreekMu   = "\xCE\xBC";

[[nodiscard]] bool isDigit( char c ) noexcept
{
    return c >= '0' && c <= '9';
}

[[nodiscard]] bool isAsciiLetter( char c ) noexcept
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
}

// Bytes of the identifier character starting at `at`: ASCII letter, '_', digit (when `allowDigit`), or the micro
// sign or Greek mu used in `µm`. Zero when there is none.
[[nodiscard]] std::size_t identifierCharLength( std::string_view source, std::size_t at, bool allowDigit ) noexcept
{
    const char c = source[at];

    if( isAsciiLetter( c ) || c == '_' || ( allowDigit && isDigit( c ) ) )
    {
        return 1;
    }

    const std::string_view rest = source.substr( at );

    if( rest.starts_with( kMicroSign ) || rest.starts_with( kGreekMu ) )
    {
        return 2;
    }

    return 0;
}

[[nodiscard]] std::size_t codePointLength( unsigned char lead ) noexcept
{
    if( lead < 0x80 )
    {
        return 1;
    }

    if( lead >= 0xC2 && lead <= 0xDF )
    {
        return 2;
    }

    if( lead >= 0xE0 && lead <= 0xEF )
    {
        return 3;
    }

    if( lead >= 0xF0 && lead <= 0xF4 )
    {
        return 4;
    }

    return 0;
}

[[nodiscard]] std::unexpected<Error> unexpectedCharacter( std::string_view source, std::size_t at )
{
    const auto        lead   = static_cast<unsigned char>( source[at] );
    const std::size_t length = codePointLength( lead );

    if( length == 0 || at + length > source.size() )
    {
        return syntaxError( source, at, std::format( "Unexpected byte 0x{:02X}", static_cast<unsigned>( lead ) ) );
    }

    if( lead < 0x20 || lead == 0x7F )
    {
        return syntaxError( source,
                            at,
                            std::format( "Unexpected control character U+{:04X}", static_cast<unsigned>( lead ) ) );
    }

    return syntaxError( source, at, std::format( "Unexpected character '{}'", source.substr( at, length ) ) );
}

// Length of the number starting at `at` (a digit, or '.' followed by a digit). A '.' belongs to the number only when
// a digit follows, and an exponent only when a digit follows the 'e' and its optional sign, so `1.e` and `2em` end
// the number early and the rest is lexed as other tokens.
[[nodiscard]] std::size_t numberLength( std::string_view source, std::size_t at ) noexcept
{
    std::size_t end = at;

    const auto digitAt = [&]( std::size_t i )
    {
        return i < source.size() && isDigit( source[i] );
    };

    while( digitAt( end ) )
    {
        ++end;
    }

    if( end < source.size() && source[end] == '.' && digitAt( end + 1 ) )
    {
        ++end;

        while( digitAt( end ) )
        {
            ++end;
        }
    }

    if( end < source.size() && ( source[end] == 'e' || source[end] == 'E' ) )
    {
        std::size_t exponent = end + 1;

        if( exponent < source.size() && ( source[exponent] == '+' || source[exponent] == '-' ) )
        {
            ++exponent;
        }

        if( digitAt( exponent ) )
        {
            end = exponent;

            while( digitAt( end ) )
            {
                ++end;
            }
        }
    }

    return end - at;
}

struct Punctuator
{
    std::string_view text;
    TokenKind        kind;
};

// Two-character operators come first so that "<=" is not read as "<" and "=".
constexpr std::array<Punctuator, 15> kPunctuators = { {
    { "<=", TokenKind::LessEqual },
    { ">=", TokenKind::GreaterEqual },
    { "==", TokenKind::EqualEqual },
    { "!=", TokenKind::NotEqual },
    { "+", TokenKind::Plus },
    { "-", TokenKind::Minus },
    { "*", TokenKind::Star },
    { "/", TokenKind::Slash },
    { "^", TokenKind::Caret },
    { "(", TokenKind::LeftParen },
    { ")", TokenKind::RightParen },
    { ",", TokenKind::Comma },
    { ".", TokenKind::Dot },
    { "<", TokenKind::Less },
    { ">", TokenKind::Greater },
} };

} // namespace

std::size_t columnOf( std::string_view source, std::size_t offset ) noexcept
{
    const std::string_view before = source.substr( 0, std::min( offset, source.size() ) );

    return 1
           + static_cast<std::size_t>( std::ranges::count_if( before,
                                                              []( char c )
                                                              {
                                                                  return ( static_cast<unsigned char>( c ) & 0xC0U )
                                                                         != 0x80U;
                                                              } ) );
}

std::unexpected<Error> syntaxError( std::string_view source, std::size_t offset, std::string_view what )
{
    const std::size_t column = columnOf( source, offset );

    return fail( ErrorCode::InvalidArgument,
                 std::format( "{} at column {}.", what, column ),
                 std::format( "{}\n{}^", source, std::string( column - 1, ' ' ) ) );
}

Result<std::vector<Token>> tokenize( std::string_view source )
{
    std::vector<Token> tokens;
    std::size_t        at = 0;

    while( at < source.size() )
    {
        const char c = source[at];

        if( c == ' ' || c == '\t' || c == '\r' || c == '\n' )
        {
            ++at;
            continue;
        }

        if( isDigit( c ) || ( c == '.' && at + 1 < source.size() && isDigit( source[at + 1] ) ) )
        {
            const std::size_t      length = numberLength( source, at );
            const std::string_view text   = source.substr( at, length );
            double                 value  = 0.0;

            // from_chars does not accept a leading '.', so ".5" is parsed as "0.5".
            const std::string padded = text.starts_with( '.' ) ? "0" + std::string( text ) : std::string( text );
            const auto [end, error]  = std::from_chars( padded.data(), padded.data() + padded.size(), value );

            if( error == std::errc::result_out_of_range )
            {
                return syntaxError( source, at, std::format( "Number '{}' is out of range", text ) );
            }

            if( error != std::errc{} || end != padded.data() + padded.size() )
            {
                return syntaxError( source, at, std::format( "Malformed number '{}'", text ) );
            }

            tokens.push_back( { TokenKind::Number, std::string( text ), { at, length }, value } );
            at += length;
            continue;
        }

        if( identifierCharLength( source, at, false ) > 0 )
        {
            std::size_t end = at;

            while( end < source.size() )
            {
                const std::size_t length = identifierCharLength( source, end, true );

                if( length == 0 )
                {
                    break;
                }

                end += length;
            }

            tokens.push_back(
                { TokenKind::Identifier, std::string( source.substr( at, end - at ) ), { at, end - at } } );
            at = end;
            continue;
        }

        const std::string_view rest  = source.substr( at );
        const auto             match = std::ranges::find_if( kPunctuators,
                                                             [&]( const Punctuator& p )
                                                             {
                                                     return rest.starts_with( p.text );
                                                             } );

        if( match == kPunctuators.end() )
        {
            return unexpectedCharacter( source, at );
        }

        tokens.push_back( { match->kind, std::string( match->text ), { at, match->text.size() } } );
        at += match->text.size();
    }

    tokens.push_back( { TokenKind::End, {}, { source.size(), 0 } } );
    return tokens;
}

} // namespace app::core::expr
