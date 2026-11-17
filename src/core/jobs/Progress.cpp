// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/jobs/Progress.cpp
/// @brief  Monotonic progress with ordered, throttled publication.

#include "core/jobs/Progress.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace app::core::jobs
{

Progress::Progress( Listener listener )
    : m_listener( std::move( listener ) )
{
}

void Progress::report( double fraction, std::string_view message )
{
    ProgressSnapshot current;
    {
        const std::scoped_lock lock( m_mutex );

        if( !std::isnan( fraction ) )
        {
            m_current.fraction = std::max( m_current.fraction, std::clamp( fraction, 0.0, 1.0 ) );
        }

        if( !message.empty() )
        {
            m_current.message = message;
        }

        current = m_current;
    }

    if( !m_listener )
    {
        return;
    }

    // Reports from several threads can reach this point out of order; the check against the last published value
    // keeps the published sequence increasing.
    const std::scoped_lock lock( m_publishMutex );

    const bool first    = !m_publishedAny;
    const bool moved    = current.fraction >= m_published.fraction + kPublishStep;
    const bool finished = current.fraction == 1.0 && m_published.fraction < 1.0;
    const bool renamed  = current.message != m_published.message && current.fraction >= m_published.fraction;

    if( first || moved || finished || renamed )
    {
        m_published    = current;
        m_publishedAny = true;
        m_listener( m_published );
    }
}

ProgressSnapshot Progress::snapshot() const
{
    const std::scoped_lock lock( m_mutex );

    return m_current;
}

} // namespace app::core::jobs
