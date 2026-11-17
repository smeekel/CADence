// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Log.h
/// @brief  Structured logging to rotating JSON-lines files in a local folder (ADR 0031). spdlog is used behind this
///         facade and is not visible to callers.
/// @req    P-23, P-03

#pragma once

#include "core/ObjectId.h"
#include "core/Result.h"

#include <chrono>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace app::core
{

/// @brief Severity of a log record, from least to most severe.
enum class LogLevel : std::uint8_t
{
    Trace,    ///< Detailed tracing, off by default.
    Debug,    ///< Developer diagnostics.
    Info,     ///< Normal events (file opened, job finished).
    Warning,  ///< Something unexpected that was handled.
    Error,    ///< An operation failed.
    Critical, ///< The application may not continue correctly.
};

/// @brief  Name of a level as written in log records.
/// @param  level  Level.
/// @return "trace", "debug", "info", "warning", "error" or "critical".
[[nodiscard]] std::string_view toString( LogLevel level ) noexcept;

/// @brief An extra key and value of a record, written under "fields".
struct LogField
{
    std::string key;   ///< Field name.
    std::string value; ///< Field value as text.
};

/// @brief One log record.
struct LogRecord
{
    std::chrono::system_clock::time_point time;                   ///< When it happened.
    LogLevel                              level = LogLevel::Info; ///< Severity.
    std::string                           module;                 ///< Module that wrote it, for example "doc".
    std::string                           message;                ///< What happened, in English.
    std::optional<ObjectId>               object;                 ///< The object concerned, if any.
    std::vector<LogField>                 fields;                 ///< Extra structured data.
};

/// @brief  The record as one line of JSON without a line break: `{"time":"2026-11-16T19:57:00.123Z","level":"info",
///         "module":"doc","message":"...","object":"<uuid>","fields":{"key":"value"}}`. "object" and "fields" are left
///         out when empty; the time is UTC with milliseconds.
/// @param  record  Record.
/// @return The JSON text.
[[nodiscard]] std::string toJsonLine( const LogRecord& record );

/// @brief Where and how much to log.
struct LogOptions
{
    std::filesystem::path folder;               ///< Log folder (injected, ADR 0024); created if missing.
    std::string           fileName = "app.log"; ///< Current file; rotated files are app.1.log, app.2.log, ...
    std::size_t maxFileBytes       = std::size_t{ 5 } << 20U; ///< Rotate before a file grows past this. @units bytes
    std::size_t maxFiles           = 5;                       ///< Rotated files kept besides the current one.
    LogLevel    minimumLevel       = LogLevel::Info;          ///< Records below this level are dropped.
};

/// @brief A log writing JSON lines to a rotating set of local files. Thread-safe. Nothing is sent over the network
///        (P-03).
class Log
{
public:
    /// @brief  Open or create the log in a folder, appending to an existing current file.
    /// @param  options  Folder, file name, rotation and level.
    /// @return The log, or ErrorCode::IoError naming the folder or file that could not be created or opened, or
    ///         ErrorCode::InvalidArgument for an empty folder, file name or size limit.
    [[nodiscard]] static Result<std::unique_ptr<Log>> open( LogOptions options );

    Log( const Log& )            = delete;
    Log& operator=( const Log& ) = delete;
    Log( Log&& )                 = delete;
    Log& operator=( Log&& )      = delete;

    /// @brief Flushes and closes the files.
    ~Log();

    /// @brief  Whether records of a level are written.
    /// @param  level  Level.
    /// @return True when @p level is at least the minimum level.
    [[nodiscard]] bool isEnabled( LogLevel level ) const noexcept;

    /// @brief Write a record if its level is enabled. A record with a default time gets the current time.
    /// @param record  Record.
    void write( LogRecord record );

    /// @brief Write a record if its level is enabled, with the current time.
    /// @param level    Severity.
    /// @param module   Module that writes it.
    /// @param message  What happened.
    /// @param object   The object concerned, if any.
    /// @param fields   Extra structured data.
    void write( LogLevel                level,
                std::string_view        module,
                std::string_view        message,
                std::optional<ObjectId> object = std::nullopt,
                std::vector<LogField>   fields = {} );

    /// @brief Write buffered records to disk.
    void flush();

    /// @brief  The log files that exist, current file first, then rotated files from newest to oldest; for "Copy
    ///         diagnostics" (S0.11.4).
    /// @return Paths.
    [[nodiscard]] std::vector<std::filesystem::path> files() const;

    /// @brief  Number of records that could not be written (for example because the disk is full).
    /// @return The count since the log was opened.
    [[nodiscard]] std::uint64_t failedWrites() const noexcept;

private:
    struct Impl;

    explicit Log( std::unique_ptr<Impl> impl ) noexcept;

    std::unique_ptr<Impl> m_impl;
};

} // namespace app::core
