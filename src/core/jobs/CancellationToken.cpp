// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/jobs/CancellationToken.cpp
/// @brief  Cancellation tokens over std::stop_source.

#include "core/jobs/CancellationToken.h"

#include <utility>

namespace app::core::jobs
{

CancellationToken::CancellationToken( std::stop_token token ) noexcept
    : m_token( std::move( token ) )
{
}

bool CancellationToken::isCancellationRequested() const noexcept
{
    return m_token.stop_requested();
}

bool CancellationToken::canBeCancelled() const noexcept
{
    return m_token.stop_possible();
}

const std::stop_token& CancellationToken::stopToken() const noexcept
{
    return m_token;
}

CancellationToken CancellationSource::token() const noexcept
{
    return CancellationToken( m_source.get_token() );
}

bool CancellationSource::requestCancellation() noexcept
{
    return m_source.request_stop();
}

bool CancellationSource::isCancellationRequested() const noexcept
{
    return m_source.stop_requested();
}

} // namespace app::core::jobs
