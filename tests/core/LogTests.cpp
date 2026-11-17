// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/LogTests.cpp
/// @brief  Unit tests for structured logging (T-01, P-23).

#include "core/Json.h"
#include "core/Log.h"

#include <chrono>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

using app::ErrorCode;
using app::core::Json;
using app::core::Log;
using app::core::LogLevel;
using app::core::LogOptions;
using app::core::LogRecord;
using app::core::ObjectIdGenerator;

namespace
{

// A fresh folder under the system temporary folder, removed with its contents at the end of the test.
class TempFolder
{
public:
    explicit TempFolder( const std::u8string& suffix = u8"" )
    {
        std::random_device random;
        m_path =
            std::filesystem::temp_directory_path() / std::format( "app-log-test-{:08x}{:08x}", random(), random() );
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
            std::fputs( "LogTests: could not remove a temporary folder.\n", stderr );
        }
    }

    [[nodiscard]] const std::filesystem::path& path() const noexcept
    {
        return m_path;
    }

private:
    std::filesystem::path m_path;
};

[[nodiscard]] std::vector<std::string> readLines( const std::filesystem::path& path )
{
    std::ifstream            stream( path, std::ios::binary );
    std::vector<std::string> lines;

    for( std::string line; std::getline( stream, line ); )
    {
        lines.push_back( line );
    }

    return lines;
}

[[nodiscard]] std::unique_ptr<Log> openLog( LogOptions options )
{
    auto log = Log::open( std::move( options ) );

    EXPECT_TRUE( log.has_value() ) << ( log ? "" : log.error().describe() );
    return log ? std::move( *log ) : nullptr;
}

} // namespace

TEST( Log, WritesOneJsonObjectPerLine_WithTheStructuredFields )
{
    const TempFolder  folder;
    ObjectIdGenerator ids    = ObjectIdGenerator::makeDeterministic( 3, 1'700'000'000'000 );
    const auto        object = ids.next();
    {
        const auto log = openLog( { .folder = folder.path() } );
        ASSERT_NE( log, nullptr );

        log->write( LogLevel::Info, "doc", "Document opened.", object, { { "path", "C:/parts/bracket.app" } } );
        log->write( LogLevel::Warning, "platform", "Slow disk." );
    }

    const auto lines = readLines( folder.path() / "app.log" );
    ASSERT_EQ( lines.size(), 2U );

    const Json first = Json::parse( lines[0] );
    EXPECT_EQ( first.at( "level" ), "info" );
    EXPECT_EQ( first.at( "module" ), "doc" );
    EXPECT_EQ( first.at( "message" ), "Document opened." );
    EXPECT_EQ( first.at( "object" ), object.toString() );
    EXPECT_EQ( first.at( "fields" ).at( "path" ), "C:/parts/bracket.app" );
    EXPECT_EQ( first.begin().key(), "time" );

    const Json second = Json::parse( lines[1] );
    EXPECT_EQ( second.at( "level" ), "warning" );
    EXPECT_FALSE( second.contains( "object" ) );
    EXPECT_FALSE( second.contains( "fields" ) );
}

TEST( Log, WritesTheTimeAsUtcWithMilliseconds )
{
    LogRecord record;
    record.time    = std::chrono::sys_days{ std::chrono::year{ 2026 } / 11 / 16 } + std::chrono::hours{ 19 }
                     + std::chrono::minutes{ 57 } + std::chrono::milliseconds{ 1'234 };
    record.module  = "core";
    record.message = "Tick.";

    EXPECT_EQ( app::core::toJsonLine( record ),
               R"({"time":"2026-11-16T19:57:01.234Z","level":"info","module":"core","message":"Tick."})" );
}

TEST( Log, KeepsEachRecordOnOneLine_WhenTheTextHasQuotesBreaksOrNonAscii )
{
    const TempFolder  folder;
    const std::string message = "Line one\nline \"two\"\twith µm and \\ backslash.";
    {
        const auto log = openLog( { .folder = folder.path() } );
        ASSERT_NE( log, nullptr );

        log->write( LogLevel::Error, "io", message, std::nullopt, { { "key \"q\"", "a\r\nb" } } );
    }

    const auto lines = readLines( folder.path() / "app.log" );
    ASSERT_EQ( lines.size(), 1U );

    const Json record = Json::parse( lines[0] );
    EXPECT_EQ( record.at( "message" ), message );
    EXPECT_EQ( record.at( "fields" ).at( "key \"q\"" ), "a\r\nb" );
}

TEST( Log, ReplacesInvalidUtf8_InsteadOfFailing )
{
    LogRecord record;
    record.message = "bad \xC3 byte";

    const Json parsed = Json::parse( app::core::toJsonLine( record ) );

    EXPECT_EQ( parsed.at( "message" ), "bad \xEF\xBF\xBD byte" );
}

TEST( Log, DropsRecordsBelowTheMinimumLevel )
{
    const TempFolder folder;
    {
        const auto log = openLog( { .folder = folder.path(), .minimumLevel = LogLevel::Warning } );
        ASSERT_NE( log, nullptr );

        EXPECT_FALSE( log->isEnabled( LogLevel::Info ) );
        EXPECT_TRUE( log->isEnabled( LogLevel::Error ) );

        log->write( LogLevel::Debug, "core", "Hidden." );
        log->write( LogLevel::Info, "core", "Hidden too." );
        log->write( LogLevel::Error, "core", "Shown." );
    }

    const auto lines = readLines( folder.path() / "app.log" );
    ASSERT_EQ( lines.size(), 1U );
    EXPECT_EQ( Json::parse( lines[0] ).at( "message" ), "Shown." );
}

TEST( Log, RotatesFiles_WhenTheSizeLimitIsReached )
{
    const TempFolder  folder;
    const std::size_t maxBytes = 1'000;
    {
        const auto log = openLog( { .folder = folder.path(), .maxFileBytes = maxBytes, .maxFiles = 2 } );
        ASSERT_NE( log, nullptr );

        for( int index = 0; index < 100; ++index )
        {
            log->write( LogLevel::Info, "core", "Record.", std::nullopt, { { "n", std::to_string( index ) } } );
        }

        const auto files = log->files();
        ASSERT_EQ( files.size(), 3U );
        EXPECT_EQ( files[0].filename(), "app.log" );
        EXPECT_EQ( files[1].filename(), "app.1.log" );
        EXPECT_EQ( files[2].filename(), "app.2.log" );
        EXPECT_EQ( log->failedWrites(), 0U );
    }

    EXPECT_FALSE( std::filesystem::exists( folder.path() / "app.3.log" ) );

    // Every file is within the limit, and reading from oldest to newest gives consecutive records ending in the last.
    std::vector<int> numbers;
    for( const char* name : { "app.2.log", "app.1.log", "app.log" } )
    {
        EXPECT_LE( std::filesystem::file_size( folder.path() / name ), maxBytes ) << name;

        for( const std::string& line : readLines( folder.path() / name ) )
        {
            numbers.push_back( std::stoi( Json::parse( line ).at( "fields" ).at( "n" ).get<std::string>() ) );
        }
    }

    ASSERT_FALSE( numbers.empty() );
    EXPECT_LT( numbers.size(), 100U );
    EXPECT_EQ( numbers.back(), 99 );
    for( std::size_t index = 1; index < numbers.size(); ++index )
    {
        EXPECT_EQ( numbers[index], numbers[index - 1] + 1 );
    }
}

TEST( Log, AppendsToTheCurrentFile_WhenOpenedAgain )
{
    const TempFolder folder;

    for( const char* message : { "First run.", "Second run." } )
    {
        const auto log = openLog( { .folder = folder.path() } );
        ASSERT_NE( log, nullptr );
        log->write( LogLevel::Info, "app", message );
    }

    const auto lines = readLines( folder.path() / "app.log" );
    ASSERT_EQ( lines.size(), 2U );
    EXPECT_EQ( Json::parse( lines[1] ).at( "message" ), "Second run." );
}

TEST( Log, WritesToAFolderWithNonAsciiCharacters )
{
    const TempFolder folder( u8"-Ünïcødé-µ-日本" );
    {
        const auto log = openLog( { .folder = folder.path() / u8"Lögs" } );
        ASSERT_NE( log, nullptr );
        log->write( LogLevel::Info, "core", "Hello." );
    }

    EXPECT_EQ( readLines( folder.path() / u8"Lögs" / "app.log" ).size(), 1U );
}

TEST( Log, ReportsIoError_WhenTheFolderCannotBeCreated )
{
    const TempFolder folder;
    std::filesystem::create_directories( folder.path() );
    std::ofstream( folder.path() / "file.txt" ) << "not a folder";

    const auto log = Log::open( { .folder = folder.path() / "file.txt" / "logs" } );

    ASSERT_FALSE( log.has_value() );
    EXPECT_EQ( log.error().code, ErrorCode::IoError );
    EXPECT_NE( log.error().detail.find( "file.txt" ), std::string::npos );
}

TEST( Log, ReportsInvalidArgument_WhenTheOptionsAreIncomplete )
{
    const auto log = Log::open( {} );

    ASSERT_FALSE( log.has_value() );
    EXPECT_EQ( log.error().code, ErrorCode::InvalidArgument );
}

TEST( Log, KeepsRecordsWhole_WhenWrittenFromManyThreads )
{
    const TempFolder folder;
    constexpr int    kThreads = 4;
    constexpr int    kRecords = 200;
    {
        const auto log = openLog( { .folder = folder.path() } );
        ASSERT_NE( log, nullptr );

        std::vector<std::jthread> threads;
        threads.reserve( kThreads );
        for( int thread = 0; thread < kThreads; ++thread )
        {
            threads.emplace_back(
                [&log, thread]
                {
                    for( int index = 0; index < kRecords; ++index )
                    {
                        log->write( LogLevel::Info,
                                    "jobs",
                                    "Step.",
                                    std::nullopt,
                                    { { "thread", std::to_string( thread ) } } );
                    }
                } );
        }
    }

    const auto lines = readLines( folder.path() / "app.log" );
    ASSERT_EQ( lines.size(), static_cast<std::size_t>( kThreads * kRecords ) );
    for( const std::string& line : lines )
    {
        EXPECT_TRUE( Json::accept( line ) ) << line;
    }
}
