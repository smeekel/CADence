// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/jobs/CancellationToken.h
/// @brief  Cooperative cancellation: a source requests it, tokens observe it.
/// @req    A-10

#pragma once

#include <stop_token>

namespace app::core::jobs
{

/// @brief Read-only view of a cancellation request. Long work polls isCancellationRequested() and returns
///        ErrorCode::Cancelled soon after it becomes true. Copies observe the same request; thread-safe.
class CancellationToken
{
public:
    /// @brief A token that is never cancelled.
    CancellationToken() noexcept = default;

    /// @brief  Whether cancellation has been requested.
    /// @return True once the source requested cancellation.
    [[nodiscard]] bool isCancellationRequested() const noexcept;

    /// @brief  Whether a source exists that can still request cancellation.
    /// @return False for a default token.
    [[nodiscard]] bool canBeCancelled() const noexcept;

    /// @brief  The underlying token, for `std::stop_callback` and `std::condition_variable_any` waits.
    /// @return The standard stop token.
    [[nodiscard]] const std::stop_token& stopToken() const noexcept;

private:
    friend class CancellationSource;

    explicit CancellationToken( std::stop_token token ) noexcept;

    std::stop_token m_token;
};

/// @brief Owner side of a cancellation request; thread-safe.
class CancellationSource
{
public:
    /// @brief  A token observing this source.
    /// @return The token.
    [[nodiscard]] CancellationToken token() const noexcept;

    /// @brief  Request cancellation; later calls have no effect.
    /// @return True if this call made the request.
    bool requestCancellation() noexcept;

    /// @brief  Whether cancellation has been requested.
    /// @return True after requestCancellation().
    [[nodiscard]] bool isCancellationRequested() const noexcept;

private:
    std::stop_source m_source;
};

} // namespace app::core::jobs
