// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Log.cpp
/// @brief  JSON-lines records and the spdlog logger with a rotating file sink on wide paths.

#include "core/Log.h"

#include "core/Json.h"
#include "core/Utf8Path.h"

#include <atomic>
#include <format>
#include <fstream>
#include <mutex>
#include <spdlog/logger.h>
#include <spdlog/sinks/base_sink.h>
#include <system_error>
#include <utility>

namespace app::core
{

namespace
{

[[nodiscard]] spdlog::level::level_enum toSpdlog( LogLevel level ) noexcept
{
    switch( level )
    {
        case LogLevel::Trace:
            return spdlog::level::trace;
        case LogLevel::Debug:
            return spdlog::level::debug;
        case LogLevel::Info:
            return spdlog::level::info;
        case LogLevel::Warning:
            return spdlog::level::warn;
        case LogLevel::Error:
            return spdlog::level::err;
        case LogLevel::Critical:
            return spdlog::level::critical;
    }

    return spdlog::level::critical;
}

// Rotating file sink on std::filesystem paths. spdlog's own rotating sink opens files through narrow paths in the
// ANSI code page, which fails for user folders with characters outside it.
class RotatingFileSink final : public spdlog::sinks::base_sink<std::mutex>
{
public:
    RotatingFileSink( std::filesystem::path folder, std::string fileName, std::size_t maxBytes, std::size_t maxFiles )
        : m_folder( std::move( folder ) )
        , m_fileName( std::move( fileName ) )
        , m_maxBytes( maxBytes )
        , m_maxFiles( maxFiles )
    {
    }

    [[nodiscard]] bool openCurrent()
    {
        const std::scoped_lock lock( mutex_ );

        return reopen();
    }

    // Path of the current file (index 0) or of rotated file `index`.
    [[nodiscard]] std::filesystem::path pathOf( std::size_t index ) const
    {
        if( index == 0 )
        {
            return m_folder / pathFromUtf8( m_fileName );
        }

        const std::size_t dot          = m_fileName.rfind( '.' );
        const bool        hasExtension = dot != std::string::npos && dot > 0;
        const std::string stem         = hasExtension ? m_fileName.substr( 0, dot ) : m_fileName;
        const std::string extension    = hasExtension ? m_fileName.substr( dot ) : std::string();

        return m_folder / pathFromUtf8( std::format( "{}.{}{}", stem, index, extension ) );
    }

    [[nodiscard]] std::size_t maxFiles() const noexcept
    {
        return m_maxFiles;
    }

    [[nodiscard]] std::uint64_t failures() const noexcept
    {
        return m_failures.load();
    }

protected:
    void sink_it_( const spdlog::details::log_msg& message ) override
    {
        const std::string_view payload( message.payload.data(), message.payload.size() );
        const std::size_t      lineBytes = payload.size() + 1;

        if( m_size > 0 && m_size + lineBytes > m_maxBytes )
        {
            rotate();
        }

        if( !m_stream.is_open() && !reopen() )
        {
            ++m_failures;
            return;
        }

        m_stream.write( payload.data(), static_cast<std::streamsize>( payload.size() ) );
        m_stream.put( '\n' );

        if( !m_stream )
        {
            ++m_failures;
            m_stream.close();
            return;
        }

        m_size += lineBytes;
    }

    void flush_() override
    {
        if( m_stream.is_open() )
        {
            m_stream.flush();
        }
    }

private:
    bool reopen()
    {
        m_stream.close();
        m_stream.clear();
        m_stream.open( pathOf( 0 ), std::ios::binary | std::ios::app );

        if( !m_stream.is_open() )
        {
            return false;
        }

        m_stream.seekp( 0, std::ios::end );

        const std::streamoff size = m_stream.tellp();

        m_size = size > 0 ? static_cast<std::size_t>( size ) : 0;
        return true;
    }

    void rotate()
    {
        m_stream.close();

        std::error_code ignored;

        std::filesystem::remove( pathOf( m_maxFiles ), ignored );

        for( std::size_t index = m_maxFiles; index > 0; --index )
        {
            if( std::filesystem::exists( pathOf( index - 1 ), ignored ) )
            {
                std::filesystem::rename( pathOf( index - 1 ), pathOf( index ), ignored );
            }
        }

        // With no rotated files kept, the current file is truncated instead.
        if( m_maxFiles == 0 )
        {
            std::filesystem::remove( pathOf( 0 ), ignored );
        }

        if( !reopen() )
        {
            ++m_failures;
        }
    }

    std::filesystem::path      m_folder;
    std::string                m_fileName;
    std::size_t                m_maxBytes;
    std::size_t                m_maxFiles;
    std::ofstream              m_stream;
    std::size_t                m_size = 0;
    std::atomic<std::uint64_t> m_failures{ 0 };
};

} // namespace

/// @brief State behind the Log facade.
struct Log::Impl
{
    LogOptions                        options;             ///< Options the log was opened with.
    std::shared_ptr<RotatingFileSink> sink;                ///< The only sink; shared with the logger.
    std::unique_ptr<spdlog::logger>   logger;              ///< Not registered with spdlog's global registry.
    std::atomic<std::uint64_t>        loggerFailures{ 0 }; ///< Errors reported by spdlog's error handler.
};

std::string_view toString( LogLevel level ) noexcept
{
    switch( level )
    {
        case LogLevel::Trace:
            return "trace";
        case LogLevel::Debug:
            return "debug";
        case LogLevel::Info:
            return "info";
        case LogLevel::Warning:
            return "warning";
        case LogLevel::Error:
            return "error";
        case LogLevel::Critical:
            return "critical";
    }

    return "critical";
}

std::string toJsonLine( const LogRecord& record )
{
    Json json = Json::object();

    json["time"]    = std::format( "{:%FT%T}Z", std::chrono::floor<std::chrono::milliseconds>( record.time ) );
    json["level"]   = toString( record.level );
    json["module"]  = record.module;
    json["message"] = record.message;

    if( record.object.has_value() )
    {
        json["object"] = record.object->toString();
    }

    if( !record.fields.empty() )
    {
        Json fields = Json::object();

        for( const LogField& field : record.fields )
        {
            fields[field.key] = field.value;
        }

        json["fields"] = std::move( fields );
    }

    // Invalid UTF-8 is replaced rather than thrown on, so a bad message can never stop logging.
    return json.dump( -1, ' ', false, Json::error_handler_t::replace );
}

Log::Log( std::unique_ptr<Impl> impl ) noexcept
    : m_impl( std::move( impl ) )
{
}

Log::~Log()
{
    m_impl->logger->flush();
}

Result<std::unique_ptr<Log>> Log::open( LogOptions options )
{
    if( options.folder.empty() || options.fileName.empty() || options.maxFileBytes == 0 )
    {
        return fail( ErrorCode::InvalidArgument, "The log needs a folder, a file name and a size limit." );
    }

    std::error_code error;

    std::filesystem::create_directories( options.folder, error );

    if( error && !std::filesystem::is_directory( options.folder ) )
    {
        return fail( ErrorCode::IoError,
                     "Cannot create the log folder.",
                     std::format( "{}: {}", utf8Of( options.folder ), error.message() ) );
    }

    auto impl = std::make_unique<Impl>();
    impl->sink =
        std::make_shared<RotatingFileSink>( options.folder, options.fileName, options.maxFileBytes, options.maxFiles );

    if( !impl->sink->openCurrent() )
    {
        return fail( ErrorCode::IoError, "Cannot open the log file.", utf8Of( impl->sink->pathOf( 0 ) ) );
    }

    impl->logger = std::make_unique<spdlog::logger>( "app", impl->sink );
    impl->logger->set_level( toSpdlog( options.minimumLevel ) );
    impl->logger->flush_on( spdlog::level::warn );
    impl->logger->set_error_handler(
        [failures = &impl->loggerFailures]( const std::string& )
        {
            ++*failures;
        } );
    impl->options = std::move( options );

    return std::unique_ptr<Log>( new Log( std::move( impl ) ) );
}

bool Log::isEnabled( LogLevel level ) const noexcept
{
    return level >= m_impl->options.minimumLevel;
}

void Log::write( LogRecord record )
{
    if( !isEnabled( record.level ) )
    {
        return;
    }

    if( record.time == std::chrono::system_clock::time_point{} )
    {
        record.time = std::chrono::system_clock::now();
    }

    m_impl->logger->log( toSpdlog( record.level ), toJsonLine( record ) );
}

void Log::write( LogLevel                level,
                 std::string_view        module,
                 std::string_view        message,
                 std::optional<ObjectId> object,
                 std::vector<LogField>   fields )
{
    if( !isEnabled( level ) )
    {
        return;
    }

    write( LogRecord{ std::chrono::system_clock::now(),
                      level,
                      std::string( module ),
                      std::string( message ),
                      object,
                      std::move( fields ) } );
}

void Log::flush()
{
    m_impl->logger->flush();
}

std::vector<std::filesystem::path> Log::files() const
{
    std::vector<std::filesystem::path> result;
    std::error_code                    error;

    for( std::size_t index = 0; index <= m_impl->sink->maxFiles(); ++index )
    {
        std::filesystem::path path = m_impl->sink->pathOf( index );

        if( std::filesystem::exists( path, error ) )
        {
            result.push_back( std::move( path ) );
        }
    }

    return result;
}

std::uint64_t Log::failedWrites() const noexcept
{
    return m_impl->sink->failures() + m_impl->loggerFailures.load();
}

} // namespace app::core
