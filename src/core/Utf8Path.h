// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Utf8Path.h
/// @brief  Conversions between file-system paths and UTF-8 text. `std::filesystem::path::string()` converts through
///         the ANSI code page on Windows and throws for characters outside it, so APP code uses these instead.
/// @req    P-04

#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace app::core
{

/// @brief  A path as UTF-8 text, for messages, logs and JSON.
/// @param  path  Path.
/// @return UTF-8 text with the native separators.
[[nodiscard]] inline std::string utf8Of( const std::filesystem::path& path )
{
    const std::u8string text = path.u8string();

    return { text.begin(), text.end() };
}

/// @brief  A path from UTF-8 text.
/// @param  text  UTF-8 text.
/// @return The path.
[[nodiscard]] inline std::filesystem::path pathFromUtf8( std::string_view text )
{
    return { std::u8string( text.begin(), text.end() ) };
}

} // namespace app::core
