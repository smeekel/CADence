// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Settings.cpp
/// @brief  Loading, saving and key paths of the settings document.

#include "core/Settings.h"

#include "core/Utf8Path.h"

#include <format>
#include <fstream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <system_error>
#include <utility>
#include <vector>

namespace app::core
{

namespace
{

constexpr std::string_view kByteOrderMark = "\xEF\xBB\xBF";

[[nodiscard]] bool isKeyCharacter( char c ) noexcept
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == '_' || c == '-';
}

[[nodiscard]] std::vector<std::string> segmentsOf( std::string_view key )
{
    std::vector<std::string> segments;

    for( const auto& segment : std::views::split( key, '.' ) )
    {
        segments.emplace_back( segment.begin(), segment.end() );
    }

    return segments;
}

[[nodiscard]] std::unexpected<Error> invalidKey( std::string_view key )
{
    return fail( ErrorCode::InvalidArgument,
                 std::format( "'{}' is not a valid settings key.", key ),
                 "Use dot-separated names of ASCII letters, digits, '_' or '-'." );
}

} // namespace

bool isValidSettingsKey( std::string_view key ) noexcept
{
    if( key.empty() || key.front() == '.' || key.back() == '.' )
    {
        return false;
    }

    char previous = '\0';

    for( const char c : key )
    {
        if( c == '.' ? previous == '.' : !isKeyCharacter( c ) )
        {
            return false;
        }

        previous = c;
    }

    return true;
}

Settings::Settings()
    : m_root( Json::object() )
{
}

Settings::Settings( Json root )
    : m_root( std::move( root ) )
{
}

Result<Settings> Settings::load( const std::filesystem::path& file )
{
    std::error_code error;

    if( !std::filesystem::exists( file, error ) )
    {
        if( error )
        {
            return fail( ErrorCode::IoError,
                         "Cannot read the settings file.",
                         std::format( "{}: {}", utf8Of( file ), error.message() ) );
        }

        return Settings();
    }

    std::ifstream stream( file, std::ios::binary );

    if( !stream )
    {
        return fail( ErrorCode::IoError, "Cannot read the settings file.", utf8Of( file ) );
    }

    const std::string text( std::istreambuf_iterator<char>( stream ), {} );

    if( stream.bad() )
    {
        return fail( ErrorCode::IoError, "Cannot read the settings file.", utf8Of( file ) );
    }

    return parse( text, utf8Of( file ) );
}

Result<Settings> Settings::parse( std::string_view text, std::string_view origin )
{
    if( text.starts_with( kByteOrderMark ) )
    {
        text.remove_prefix( kByteOrderMark.size() );
    }

    try
    {
        Json root = Json::parse( text );

        if( !root.is_object() )
        {
            return fail( ErrorCode::FormatError,
                         "The settings file must contain a JSON object.",
                         std::format( "{}: found {}", origin, root.type_name() ) );
        }

        return Settings( std::move( root ) );
    }
    catch( const Json::exception& error )
    {
        return fail( ErrorCode::FormatError,
                     "The settings file is not valid JSON.",
                     std::format( "{}: {}", origin, error.what() ) );
    }
}

Result<> Settings::save( const std::filesystem::path& file ) const
{
    const auto ioError = [&file]( std::string_view what )
    {
        return fail( ErrorCode::IoError,
                     "Cannot write the settings file.",
                     std::format( "{}: {}", utf8Of( file ), what ) );
    };

    std::error_code error;

    if( file.has_parent_path() )
    {
        std::filesystem::create_directories( file.parent_path(), error );

        if( error )
        {
            return ioError( error.message() );
        }
    }

    std::filesystem::path temporary  = file;
    temporary                       += ".tmp";

    const std::string text = toText();
    {
        std::ofstream stream( temporary, std::ios::binary | std::ios::trunc );
        stream.write( text.data(), static_cast<std::streamsize>( text.size() ) );
        stream.close();

        if( !stream )
        {
            std::filesystem::remove( temporary, error );
            return ioError( "writing the temporary file failed" );
        }
    }

    std::filesystem::rename( temporary, file, error );

    if( error )
    {
        const std::string message = error.message();
        std::filesystem::remove( temporary, error );
        return ioError( message );
    }

    return ok();
}

std::string Settings::toText() const
{
    return m_root.dump( 2, ' ', false, Json::error_handler_t::replace ) + "\n";
}

bool Settings::contains( std::string_view key ) const
{
    return find( key ).has_value();
}

Result<Json> Settings::find( std::string_view key ) const
{
    if( !isValidSettingsKey( key ) )
    {
        return invalidKey( key );
    }

    const Json* node = &m_root;

    for( const std::string& segment : segmentsOf( key ) )
    {
        if( !node->is_object() )
        {
            return fail( ErrorCode::NotFound, std::format( "The setting '{}' is not set.", key ) );
        }

        const auto found = node->find( segment );

        if( found == node->end() )
        {
            return fail( ErrorCode::NotFound, std::format( "The setting '{}' is not set.", key ) );
        }

        node = &*found;
    }

    return *node;
}

Result<> Settings::set( std::string_view key, Json value )
{
    if( !isValidSettingsKey( key ) )
    {
        return invalidKey( key );
    }

    const std::vector<std::string> segments = segmentsOf( key );
    Json*                          node     = &m_root;
    std::string                    path;

    for( std::size_t index = 0; index + 1 < segments.size(); ++index )
    {
        path += ( index == 0 ? "" : "." ) + segments[index];

        Json& child = ( *node )[segments[index]];

        if( child.is_null() )
        {
            child = Json::object();
        }
        else if( !child.is_object() )
        {
            return fail( ErrorCode::Conflict,
                         std::format( "Cannot set '{}': '{}' holds a value that is not an object.", key, path ) );
        }

        node = &child;
    }

    ( *node )[segments.back()] = std::move( value );
    return ok();
}

bool Settings::remove( std::string_view key )
{
    if( !isValidSettingsKey( key ) )
    {
        return false;
    }

    const std::vector<std::string> segments = segmentsOf( key );
    Json*                          node     = &m_root;

    for( std::size_t index = 0; index + 1 < segments.size(); ++index )
    {
        const auto found = node->find( segments[index] );

        if( found == node->end() || !found->is_object() )
        {
            return false;
        }

        node = &*found;
    }

    return node->erase( segments.back() ) > 0;
}

const Json& Settings::json() const noexcept
{
    return m_root;
}

std::unexpected<Error> Settings::wrongType( std::string_view key, const Json& value )
{
    return fail( ErrorCode::FormatError,
                 std::format( "The setting '{}' has the wrong type.", key ),
                 std::format( "found {}", value.type_name() ) );
}

} // namespace app::core
