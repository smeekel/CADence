// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Settings.h
/// @brief  User settings as an open JSON file at an injected path (ADR 0024). Keys APP does not know are kept and
///         written back, so settings from newer versions or edited by hand survive.
/// @req    F-10, U-10, P-04

#pragma once

#include "core/Json.h"
#include "core/Result.h"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

namespace app::core
{

/// @brief  Whether a settings key is valid: dot-separated segments of ASCII letters, digits, '_' and '-', for example
///         "library.userFolder". Each segment names a member of a nested JSON object.
/// @param  key  Key to check.
/// @return True when valid.
[[nodiscard]] bool isValidSettingsKey( std::string_view key ) noexcept;

/// @brief A settings document: a JSON object whose members keep their order. Not thread-safe; its owner (`ui` or
///        `cli`) serialises access.
class Settings
{
public:
    /// @brief Empty settings.
    Settings();

    /// @brief  Read settings from a file. A missing file gives empty settings, so the first run needs no file.
    /// @param  file  Settings file.
    /// @return The settings; ErrorCode::FormatError if the file is not a JSON object (the detail gives the path and
    ///         the position); ErrorCode::IoError if it exists but cannot be read.
    [[nodiscard]] static Result<Settings> load( const std::filesystem::path& file );

    /// @brief  Parse settings from text; a leading UTF-8 byte order mark is ignored.
    /// @param  text    JSON text.
    /// @param  origin  Where the text came from, for error details.
    /// @return The settings, or ErrorCode::FormatError.
    [[nodiscard]] static Result<Settings> parse( std::string_view text, std::string_view origin = "settings" );

    /// @brief  Write the settings as UTF-8 JSON with two-space indentation, through a temporary file renamed over the
    ///         target so a failed write leaves the old file intact. Creates the parent folder.
    /// @param  file  Settings file.
    /// @return Success or ErrorCode::IoError.
    [[nodiscard]] Result<> save( const std::filesystem::path& file ) const;

    /// @brief  The text save() writes.
    /// @return JSON text ending in a line break.
    [[nodiscard]] std::string toText() const;

    /// @brief  Whether a key has a value.
    /// @param  key  Settings key.
    /// @return True if present.
    [[nodiscard]] bool contains( std::string_view key ) const;

    /// @brief  The value of a key.
    /// @param  key  Settings key.
    /// @return The value; ErrorCode::NotFound if absent; ErrorCode::InvalidArgument for an invalid key.
    [[nodiscard]] Result<Json> find( std::string_view key ) const;

    /// @brief  The value of a key as @p T (bool, std::int64_t, double, std::string or Json).
    /// @param  key  Settings key.
    /// @return The value; ErrorCode::NotFound if absent; ErrorCode::FormatError if it has another type.
    template <typename T>
    [[nodiscard]] Result<T> get( std::string_view key ) const
    {
        return find( key ).and_then(
            [key]( const Json& value )
            {
                return convert<T>( key, value );
            } );
    }

    /// @brief  The value of a key as @p T, or a default when it is absent or has another type.
    /// @param  key       Settings key.
    /// @param  fallback  Default.
    /// @return The value or @p fallback.
    template <typename T>
    [[nodiscard]] T getOr( std::string_view key, T fallback ) const
    {
        Result<T> value = get<T>( key );

        return value ? std::move( *value ) : std::move( fallback );
    }

    /// @brief  Set the value of a key, creating the objects on its path.
    /// @param  key    Settings key.
    /// @param  value  Value.
    /// @return Success; ErrorCode::InvalidArgument for an invalid key; ErrorCode::Conflict if a segment of the path
    ///         holds a value that is not an object.
    [[nodiscard]] Result<> set( std::string_view key, Json value );

    /// @brief  Remove a key; objects left empty stay.
    /// @param  key  Settings key.
    /// @return True if it was present.
    bool remove( std::string_view key );

    /// @brief  The whole document.
    /// @return The JSON object.
    [[nodiscard]] const Json& json() const noexcept;

private:
    explicit Settings( Json root );

    template <typename T>
    [[nodiscard]] static Result<T> convert( std::string_view key, const Json& value )
    {
        if constexpr( std::is_same_v<T, Json> )
        {
            return value;
        }
        else
        {
            static_assert( std::is_same_v<T, bool> || std::is_same_v<T, std::int64_t> || std::is_same_v<T, double>
                               || std::is_same_v<T, std::string>,
                           "Settings::get supports bool, std::int64_t, double, std::string and Json." );

            bool matches = false;

            if constexpr( std::is_same_v<T, bool> )
            {
                matches = value.is_boolean();
            }
            else if constexpr( std::is_same_v<T, std::int64_t> )
            {
                matches = value.is_number_integer()
                          && !( value.is_number_unsigned()
                                && value.get<std::uint64_t>()
                                       > static_cast<std::uint64_t>( std::numeric_limits<std::int64_t>::max() ) );
            }
            else if constexpr( std::is_same_v<T, double> )
            {
                matches = value.is_number();
            }
            else
            {
                matches = value.is_string();
            }

            if( !matches )
            {
                return wrongType( key, value );
            }

            return value.get<T>();
        }
    }

    [[nodiscard]] static std::unexpected<Error> wrongType( std::string_view key, const Json& value );

    Json m_root;
};

} // namespace app::core
