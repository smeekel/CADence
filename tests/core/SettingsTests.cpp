// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/SettingsTests.cpp
/// @brief  Unit tests for user settings and the known-folder interface (T-01, F-10, U-10, P-04).

#include "core/IKnownFolders.h"
#include "core/Json.h"
#include "core/Settings.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <random>
#include <set>
#include <string>
#include <system_error>

using app::ErrorCode;
using app::core::FixedKnownFolders;
using app::core::isValidSettingsKey;
using app::core::Json;
using app::core::kAllKnownFolders;
using app::core::KnownFolder;
using app::core::Settings;

namespace
{

// A fresh folder under the system temporary folder, removed with its contents at the end of the test.
class TempFolder
{
public:
    explicit TempFolder( const std::u8string& suffix = u8"" )
    {
        std::random_device random;
        m_path  = std::filesystem::temp_directory_path()
                  / std::format( "app-settings-test-{:08x}{:08x}", random(), random() );
        m_path += suffix;
    }

    TempFolder( const TempFolder& )            = delete;
    TempFolder& operator=( const TempFolder& ) = delete;
    TempFolder( TempFolder&& )                 = delete;
    TempFolder& operator=( TempFolder&& )      = delete;

    ~TempFolder()
    {
        try
        {
            std::error_code ignored;
            std::filesystem::remove_all( m_path, ignored );
        }
        catch( const std::exception& )
        {
            std::fputs( "SettingsTests: could not remove a temporary folder.\n", stderr );
        }
    }

    [[nodiscard]] const std::filesystem::path& path() const noexcept
    {
        return m_path;
    }

private:
    std::filesystem::path m_path;
};

void writeFile( const std::filesystem::path& file, std::string_view text )
{
    std::filesystem::create_directories( file.parent_path() );
    std::ofstream stream( file, std::ios::binary );
    stream.write( text.data(), static_cast<std::streamsize>( text.size() ) );
}

[[nodiscard]] std::string readFile( const std::filesystem::path& file )
{
    std::ifstream stream( file, std::ios::binary );

    return { std::istreambuf_iterator<char>( stream ), {} };
}

} // namespace

TEST( Settings, StartsEmpty_WhenTheFileDoesNotExist )
{
    const TempFolder folder;

    const auto settings = Settings::load( folder.path() / "settings.json" );

    ASSERT_TRUE( settings.has_value() ) << settings.error().describe();
    EXPECT_TRUE( settings->json().is_object() );
    EXPECT_TRUE( settings->json().empty() );
    EXPECT_FALSE( std::filesystem::exists( folder.path() ) );
}

TEST( Settings, RoundTripsValues_ThroughSaveAndLoad )
{
    const TempFolder folder;
    const auto       file = folder.path() / "nested" / "settings.json";

    Settings settings;
    ASSERT_TRUE( settings.set( "ui.theme", "dark" ).has_value() );
    ASSERT_TRUE( settings.set( "ui.recentFiles.limit", 12 ).has_value() );
    ASSERT_TRUE( settings.set( "units.decimals", 3.5 ).has_value() );
    ASSERT_TRUE( settings.set( "library.readOnlyFolders", Json::array( { "C:/team/libs" } ) ).has_value() );
    ASSERT_TRUE( settings.set( "autosave", true ).has_value() );
    ASSERT_TRUE( settings.save( file ).has_value() );

    EXPECT_FALSE( std::filesystem::exists( folder.path() / "nested" / "settings.json.tmp" ) );

    const auto loaded = Settings::load( file );
    ASSERT_TRUE( loaded.has_value() ) << loaded.error().describe();
    EXPECT_EQ( loaded->get<std::string>( "ui.theme" ), "dark" );
    EXPECT_EQ( loaded->get<std::int64_t>( "ui.recentFiles.limit" ), 12 );
    EXPECT_DOUBLE_EQ( loaded->get<double>( "units.decimals" ).value(), 3.5 );
    EXPECT_EQ( loaded->get<bool>( "autosave" ), true );
    EXPECT_EQ( loaded->get<Json>( "library.readOnlyFolders" )->at( 0 ), "C:/team/libs" );
    EXPECT_EQ( loaded->json(), settings.json() );
}

TEST( Settings, KeepsUnknownKeysAndTheirOrder_WhenSavedAgain )
{
    const TempFolder folder;
    const auto       file = folder.path() / "settings.json";
    writeFile( file, R"({
  "zeta": { "fromNewerVersion": [1, 2, {"deep": null}] },
  "ui": { "theme": "light", "pluginOption": "x" },
  "alpha": 1
}
)" );

    auto settings = Settings::load( file );
    ASSERT_TRUE( settings.has_value() ) << settings.error().describe();
    ASSERT_TRUE( settings->set( "ui.theme", "dark" ).has_value() );
    ASSERT_TRUE( settings->save( file ).has_value() );

    EXPECT_EQ( readFile( file ), R"({
  "zeta": {
    "fromNewerVersion": [
      1,
      2,
      {
        "deep": null
      }
    ]
  },
  "ui": {
    "theme": "dark",
    "pluginOption": "x"
  },
  "alpha": 1
}
)" );
}

TEST( Settings, ReportsFormatError_WhenTheFileIsNotAJsonObject )
{
    const TempFolder folder;
    const auto       broken = folder.path() / "broken.json";
    const auto       array  = folder.path() / "array.json";
    writeFile( broken, "{ \"ui\": " );
    writeFile( array, "[1, 2]" );

    const auto first = Settings::load( broken );
    ASSERT_FALSE( first.has_value() );
    EXPECT_EQ( first.error().code, ErrorCode::FormatError );
    EXPECT_NE( first.error().detail.find( "broken.json" ), std::string::npos );

    const auto second = Settings::load( array );
    ASSERT_FALSE( second.has_value() );
    EXPECT_EQ( second.error().code, ErrorCode::FormatError );
    EXPECT_NE( second.error().detail.find( "array" ), std::string::npos );
}

TEST( Settings, AcceptsAByteOrderMark )
{
    const auto settings = Settings::parse( "\xEF\xBB\xBF{\"a\": 1}" );

    ASSERT_TRUE( settings.has_value() );
    EXPECT_EQ( settings->get<std::int64_t>( "a" ), 1 );
}

TEST( Settings, ReportsMissingKeysAndWrongTypes )
{
    auto settings = Settings::parse( R"({"ui": {"theme": "dark", "scale": 1.25}, "count": 18446744073709551615})" );
    ASSERT_TRUE( settings.has_value() );

    const auto missing = settings->get<std::string>( "ui.font" );
    ASSERT_FALSE( missing.has_value() );
    EXPECT_EQ( missing.error().code, ErrorCode::NotFound );
    EXPECT_EQ( missing.error().message, "The setting 'ui.font' is not set." );

    const auto wrong = settings->get<bool>( "ui.theme" );
    ASSERT_FALSE( wrong.has_value() );
    EXPECT_EQ( wrong.error().code, ErrorCode::FormatError );

    EXPECT_FALSE( settings->get<std::int64_t>( "ui.scale" ).has_value() );
    EXPECT_FALSE( settings->get<std::int64_t>( "count" ).has_value() );
    EXPECT_FALSE( settings->get<std::string>( "ui.theme.inner" ).has_value() );

    EXPECT_EQ( settings->getOr<std::string>( "ui.theme", "light" ), "dark" );
    EXPECT_EQ( settings->getOr<std::string>( "ui.font", "Segoe UI" ), "Segoe UI" );
    EXPECT_EQ( settings->getOr<bool>( "ui.theme", false ), false );
    EXPECT_TRUE( settings->contains( "ui.scale" ) );
    EXPECT_FALSE( settings->contains( "ui.font" ) );
}

TEST( Settings, RejectsInvalidKeys_AndPathsThroughValues )
{
    Settings settings;

    for( const char* key : { "", ".a", "a.", "a..b", "a b", "é", "a/b" } )
    {
        EXPECT_FALSE( isValidSettingsKey( key ) ) << key;
        const auto set = settings.set( key, 1 );
        ASSERT_FALSE( set.has_value() ) << key;
        EXPECT_EQ( set.error().code, ErrorCode::InvalidArgument );
    }

    EXPECT_TRUE( isValidSettingsKey( "library.user_folder-2" ) );

    ASSERT_TRUE( settings.set( "a.b", 1 ).has_value() );
    const auto through = settings.set( "a.b.c", 2 );
    ASSERT_FALSE( through.has_value() );
    EXPECT_EQ( through.error().code, ErrorCode::Conflict );
    EXPECT_EQ( through.error().message, "Cannot set 'a.b.c': 'a.b' holds a value that is not an object." );
    EXPECT_EQ( settings.get<std::int64_t>( "a.b" ), 1 );
}

TEST( Settings, RemovesKeys )
{
    auto settings = Settings::parse( R"({"ui": {"theme": "dark", "scale": 2}})" );
    ASSERT_TRUE( settings.has_value() );

    EXPECT_TRUE( settings->remove( "ui.theme" ) );
    EXPECT_FALSE( settings->remove( "ui.theme" ) );
    EXPECT_FALSE( settings->remove( "ui.scale.inner" ) );
    EXPECT_FALSE( settings->remove( "nope.x" ) );
    EXPECT_EQ( settings->json(), Json::parse( R"({"ui": {"scale": 2}})" ) );
}

TEST( Settings, SavesToAFolderWithNonAsciiCharacters )
{
    const TempFolder folder( u8"-Ünïcødé-日本" );
    const auto       file = folder.path() / u8"Réglages" / "settings.json";

    Settings settings;
    ASSERT_TRUE( settings.set( "user.name", "Zoë" ).has_value() );
    ASSERT_TRUE( settings.save( file ).has_value() );

    const auto loaded = Settings::load( file );
    ASSERT_TRUE( loaded.has_value() );
    EXPECT_EQ( loaded->get<std::string>( "user.name" ), "Zoë" );
}

TEST( Settings, ReportsIoError_WhenTheFolderCannotBeCreated )
{
    const TempFolder folder;
    writeFile( folder.path() / "file.txt", "not a folder" );

    const auto saved = Settings().save( folder.path() / "file.txt" / "settings.json" );

    ASSERT_FALSE( saved.has_value() );
    EXPECT_EQ( saved.error().code, ErrorCode::IoError );
}

TEST( KnownFolders, GivesEachFolderItsOwnPath_UnderTheRoot )
{
    const std::filesystem::path     root = u8"C:/Tést/profile";
    FixedKnownFolders               folders( root );
    std::set<std::filesystem::path> seen;

    for( const KnownFolder folder : kAllKnownFolders )
    {
        const auto path = folders.path( folder );
        ASSERT_TRUE( path.has_value() );
        EXPECT_EQ( path->parent_path(), root );
        EXPECT_TRUE( seen.insert( *path ).second ) << toString( folder );
    }

    EXPECT_EQ( folders.path( KnownFolder::UserLibraries ).value(), root / "userLibraries" );

    folders.set( KnownFolder::GlobalLibraries, "D:/seed" );
    EXPECT_EQ( folders.path( KnownFolder::GlobalLibraries ).value(), std::filesystem::path( "D:/seed" ) );
}
