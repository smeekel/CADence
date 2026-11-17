// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/jobs/Progress.h
/// @brief  Monotonic progress of one job, with a throttled listener for the status bar.
/// @req    A-10

#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <string_view>

namespace app::core::jobs
{

/// @brief Progress at one moment.
struct ProgressSnapshot
{
    double      fraction = 0.0; ///< Completed fraction from 0 to 1.
    std::string message;        ///< What the job is doing.
};

/// @brief Progress of one job. The fraction only grows: reports below the current value keep the current value, so a
///        progress bar never moves backwards. Thread-safe.
class Progress
{
public:
    /// @brief Called with each published change, in increasing order of fraction. Not called with a lock that
    ///        snapshot() needs, so a listener may read the progress.
    using Listener = std::function<void( const ProgressSnapshot& )>;

    /// @brief Smallest increase of the fraction that is published to the listener; a change of message, the first
    ///        report and reaching 1 are always published.
    static constexpr double kPublishStep = 0.001;

    /// @brief Progress starting at 0 with no message.
    /// @param listener  Receives published changes; may be empty.
    explicit Progress( Listener listener = {} );

    /// @brief Report progress.
    /// @param fraction  Completed fraction; clamped to [0, 1]; NaN and values below the current fraction keep the
    ///                  current fraction.
    /// @param message   What the job is doing; empty keeps the current message.
    void report( double fraction, std::string_view message = {} );

    /// @brief  The current progress.
    /// @return Fraction and message.
    [[nodiscard]] ProgressSnapshot snapshot() const;

private:
    mutable std::mutex m_mutex;
    ProgressSnapshot   m_current;
    std::mutex         m_publishMutex;
    ProgressSnapshot   m_published;
    bool               m_publishedAny = false;
    Listener           m_listener;
};

} // namespace app::core::jobs
