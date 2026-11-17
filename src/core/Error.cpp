// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Error.cpp
/// @brief  Names and descriptions of errors.

#include "core/Error.h"

#include <utility>

namespace app
{

std::string_view toString( ErrorCode code ) noexcept
{
    switch( code )
    {
        case ErrorCode::InvalidArgument:
            return "invalid-argument";
        case ErrorCode::NotFound:
            return "not-found";
        case ErrorCode::Conflict:
            return "conflict";
        case ErrorCode::GeometryFailure:
            return "geometry-failure";
        case ErrorCode::FormatError:
            return "format-error";
        case ErrorCode::IoError:
            return "io-error";
        case ErrorCode::Unsupported:
            return "unsupported";
        case ErrorCode::Cancelled:
            return "cancelled";
        case ErrorCode::Internal:
            return "internal";
    }

    return "unknown";
}

std::string Error::describe() const
{
    std::string text = std::string( toString( code ) ) + ": " + message;

    if( !detail.empty() )
    {
        text += " (" + detail + ")";
    }

    return text;
}

std::unexpected<Error>
fail( ErrorCode code, std::string message, std::string detail, std::vector<core::ObjectId> objects )
{
    return std::unexpected<Error>( Error{ code, std::move( message ), std::move( detail ), std::move( objects ) } );
}

} // namespace app
