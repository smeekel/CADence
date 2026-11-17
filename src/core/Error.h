// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Error.h
/// @brief  The error value every module returns across its public API (ADR 0042).
/// @req    A-02, A-03

#pragma once

#include "core/ObjectId.h"

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace app
{

/// @brief Category of an error. The names returned by toString() are stable: they appear in CLI output, journals and
///        test expectations.
enum class ErrorCode : std::uint8_t
{
    InvalidArgument, ///< A parameter is missing, has the wrong type, or is out of range (including unit mismatches).
    NotFound,        ///< A named or identified object does not exist.
    Conflict,        ///< The request contradicts existing state (over-constrained sketch, link cycle, duplicate).
    GeometryFailure, ///< The geometry kernel could not perform an operation.
    FormatError,     ///< A file or JSON value does not match its format or schema.
    IoError,         ///< Reading or writing a file or folder failed.
    Unsupported,     ///< The operation or format version is not supported.
    Cancelled,       ///< A job was cancelled by the user or by its owner.
    Internal         ///< A programming error escaped as a value; always a defect.
};

/// @brief  Stable kebab-case name of @p code, for example "invalid-argument".
/// @param  code  Error category.
/// @return Name of the category; "unknown" only for a value outside the enumeration.
[[nodiscard]] std::string_view toString( ErrorCode code ) noexcept;

/// @brief An error with a category, a user-readable message, developer detail and the objects it concerns.
///
/// Errors are values: they are returned inside app::Result and never thrown across a module's public API.
struct Error
{
    ErrorCode                   code = ErrorCode::Internal; ///< Category, used by callers to react.
    std::string                 message; ///< User-readable English sentence, shown in the UI and CLI.
    std::string                 detail;  ///< Developer detail: file, JSON path, kernel status, parameter name.
    std::vector<core::ObjectId> objects; ///< Objects the UI highlights and the Messages panel locates.

    /// @brief  One-line description: "<code>: <message>", followed by " (<detail>)" when there is detail.
    /// @return Text for logs and CLI output.
    [[nodiscard]] std::string describe() const;

    /// @brief Two errors are equal when code, message, detail and objects are equal.
    [[nodiscard]] friend bool operator==( const Error&, const Error& ) = default;
};

/// @brief  Build an error as the failure value of a Result: `return app::fail( ErrorCode::NotFound, "..." );`.
/// @param  code     Error category.
/// @param  message  User-readable message.
/// @param  detail   Optional developer detail.
/// @param  objects  Optional objects to highlight.
/// @return The error wrapped in `std::unexpected`, convertible to any `Result<T>`.
[[nodiscard]] std::unexpected<Error>
fail( ErrorCode code, std::string message, std::string detail = {}, std::vector<core::ObjectId> objects = {} );

} // namespace app
