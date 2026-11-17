// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/EventBus.cpp
/// @brief  Subscription bookkeeping and dispatch of the event bus.

#include "core/EventBus.h"

#include <algorithm>
#include <utility>

namespace app::core
{

namespace
{

thread_local int t_dispatchDepth = 0;

class DispatchScope
{
public:
    DispatchScope() noexcept
    {
        ++t_dispatchDepth;
    }

    DispatchScope( const DispatchScope& )            = delete;
    DispatchScope& operator=( const DispatchScope& ) = delete;

    ~DispatchScope()
    {
        --t_dispatchDepth;
    }
};

// Drops released slots; called with the bus mutex held.
void prune( std::vector<std::shared_ptr<detail::EventSlot>>& slots )
{
    std::erase_if( slots,
                   []( const std::shared_ptr<detail::EventSlot>& slot )
                   {
                       return !slot->active.load();
                   } );
}

} // namespace

Subscription::Subscription( std::weak_ptr<detail::EventSlot> slot ) noexcept
    : m_slot( std::move( slot ) )
{
}

Subscription& Subscription::operator=( Subscription&& other ) noexcept
{
    if( this != &other )
    {
        reset();
        m_slot = std::move( other.m_slot );
    }

    return *this;
}

Subscription::~Subscription()
{
    reset();
}

void Subscription::reset()
{
    if( const std::shared_ptr<detail::EventSlot> slot = m_slot.lock() )
    {
        std::unique_lock lock( slot->mutex );

        slot->active.store( false );
        slot->idle.wait( lock,
                         [&]
                         {
                             return std::ranges::all_of( slot->callers,
                                                         []( std::thread::id id )
                                                         {
                                                             return id == std::this_thread::get_id();
                                                         } );
                         } );
    }

    m_slot.reset();
}

bool Subscription::isActive() const noexcept
{
    const std::shared_ptr<detail::EventSlot> slot = m_slot.lock();

    return slot != nullptr && slot->active.load();
}

Subscription EventBus::add( std::size_t kind, std::function<void( const void* )> call )
{
    auto slot = std::make_shared<detail::EventSlot>();

    slot->call = std::move( call );

    const std::scoped_lock lock( m_mutex );

    prune( m_slots.at( kind ) );
    m_slots.at( kind ).push_back( slot );
    return Subscription( slot );
}

void EventBus::dispatch( std::size_t kind, const void* event ) const
{
    std::vector<std::shared_ptr<detail::EventSlot>> snapshot;

    {
        const std::scoped_lock lock( m_mutex );

        prune( m_slots.at( kind ) );
        snapshot = m_slots.at( kind );
    }

    const DispatchScope scope;

    const std::thread::id self = std::this_thread::get_id();

    for( const std::shared_ptr<detail::EventSlot>& slot : snapshot )
    {
        {
            const std::scoped_lock lock( slot->mutex );

            if( !slot->active.load() )
            {
                continue;
            }

            slot->callers.push_back( self );
        }

        slot->call( event );

        {
            const std::scoped_lock lock( slot->mutex );

            slot->callers.erase( std::ranges::find( slot->callers, self ) );
        }

        slot->idle.notify_all();
    }
}

std::size_t EventBus::countActive( std::size_t kind ) const
{
    const std::scoped_lock lock( m_mutex );

    return static_cast<std::size_t>( std::ranges::count_if( m_slots.at( kind ),
                                                            []( const std::shared_ptr<detail::EventSlot>& slot )
                                                            {
                                                                return slot->active.load();
                                                            } ) );
}

bool EventBus::isDispatching() noexcept
{
    return t_dispatchDepth > 0;
}

} // namespace app::core
