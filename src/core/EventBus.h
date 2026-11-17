// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/EventBus.h
/// @brief  Application events (document changes, regeneration state, job progress) and the bus that delivers them.
/// @req    A-03, A-07

#pragma once

#include "core/ObjectId.h"

#include <array>
#include <atomic>
#include <concepts>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace app::core
{

/// @brief Objects added, modified and removed by one change to a document.
struct ChangeSet
{
    std::vector<ObjectId> added;    ///< New objects.
    std::vector<ObjectId> modified; ///< Changed objects.
    std::vector<ObjectId> removed;  ///< Deleted objects.

    /// @brief Change sets are equal when all three lists are equal.
    [[nodiscard]] friend bool operator==( const ChangeSet&, const ChangeSet& ) = default;
};

/// @brief A document was changed by a command (or its undo or redo).
struct DocumentChanged
{
    ObjectId  documentId; ///< The changed document.
    ChangeSet changes;    ///< What changed.
};

/// @brief State of a node of the regeneration graph (A-07).
enum class RegenState : std::uint8_t
{
    UpToDate,     ///< Output matches the inputs.
    OutOfDate,    ///< An input changed, or an upstream node failed; regeneration pending.
    Regenerating, ///< Being recomputed.
    Error,        ///< The last regeneration failed; the last good output is kept for display (P-21).
};

/// @brief A regeneration node changed state.
struct RegenStateChanged
{
    ObjectId   nodeId;                        ///< The node.
    RegenState state = RegenState::OutOfDate; ///< Its new state.
};

/// @brief Progress of a background job (A-10).
struct JobProgress
{
    std::uint64_t jobId    = 0;   ///< The job.
    double        fraction = 0.0; ///< Completed fraction from 0 to 1.
    std::string   message;        ///< What it is doing, for the status bar.
};

/// @brief The event types the bus carries. UI-only events (selection) live in `ui`.
template <typename E>
concept BusEvent =
    std::same_as<E, DocumentChanged> || std::same_as<E, RegenStateChanged> || std::same_as<E, JobProgress>;

/// @cond
namespace detail
{

// No lock is held while the handler runs; `callers` lists the threads inside it so that reset() can wait for the
// others without deadlocking a handler that releases itself.
struct EventSlot
{
    std::function<void( const void* )> call;
    std::mutex                         mutex;
    std::condition_variable            idle;
    std::vector<std::thread::id>       callers;
    std::atomic<bool>                  active{ true };
};

} // namespace detail

/// @endcond

/// @brief A handler registration. Releasing it (reset() or destruction) stops further calls; it may outlive the bus.
class Subscription
{
public:
    /// @brief An empty subscription.
    Subscription() noexcept = default;

    Subscription( const Subscription& )            = delete;
    Subscription& operator=( const Subscription& ) = delete;

    /// @brief Take over another subscription.
    /// @param other  Subscription to move from; left empty.
    Subscription( Subscription&& other ) noexcept = default;

    /// @brief  Release this subscription and take over another.
    /// @param  other  Subscription to move from; left empty.
    /// @return This subscription.
    Subscription& operator=( Subscription&& other ) noexcept;

    /// @brief Releases the subscription.
    ~Subscription();

    /// @brief Stop calling the handler. If it is running on other threads, waits until those calls return; from
    ///        inside the handler itself it returns at once. Safe to call more than once. Two handlers running on
    ///        different threads must not release each other's subscriptions, since each would wait for the other.
    void reset();

    /// @brief  Whether the handler can still be called.
    /// @return False after reset(), after a move from, and for an empty subscription.
    [[nodiscard]] bool isActive() const noexcept;

private:
    friend class EventBus;

    explicit Subscription( std::weak_ptr<detail::EventSlot> slot ) noexcept;

    std::weak_ptr<detail::EventSlot> m_slot;
};

/// @brief Publish-subscribe bus for application events.
///
/// Handlers run on the publishing thread, in subscription order, and receive the event as const. They must not change
/// documents: they post commands instead, and the command bus refuses to run a command while isDispatching() is true
/// (A-03). Handlers may publish further events and may subscribe or unsubscribe; a handler added during a dispatch is
/// first called by the next publish. Thread-safe.
class EventBus
{
public:
    /// @brief An empty bus.
    EventBus() = default;

    EventBus( const EventBus& )            = delete;
    EventBus& operator=( const EventBus& ) = delete;
    EventBus( EventBus&& )                 = delete;
    EventBus& operator=( EventBus&& )      = delete;
    ~EventBus()                            = default;

    /// @brief  Register a handler for one event type.
    /// @tparam E        Event type.
    /// @param  handler  Called for each published @p E.
    /// @return The subscription; the handler is called until it is released.
    template <BusEvent E>
    [[nodiscard]] Subscription subscribe( std::function<void( const E& )> handler )
    {
        return add( kindOf<E>(),
                    [handler = std::move( handler )]( const void* event )
                    {
                        handler( *static_cast<const E*>( event ) );
                    } );
    }

    /// @brief Deliver an event to every active handler of its type.
    /// @tparam E      Event type.
    /// @param event  The event.
    template <BusEvent E>
    void publish( const E& event ) const
    {
        dispatch( kindOf<E>(), &event );
    }

    /// @brief  Number of active handlers for one event type.
    /// @tparam E  Event type.
    /// @return The count.
    template <BusEvent E>
    [[nodiscard]] std::size_t subscriberCount() const
    {
        return countActive( kindOf<E>() );
    }

    /// @brief  Whether the calling thread is inside an event handler of any bus.
    /// @return True during a handler call.
    [[nodiscard]] static bool isDispatching() noexcept;

private:
    static constexpr std::size_t kKinds = 3;

    template <BusEvent E>
    [[nodiscard]] static constexpr std::size_t kindOf() noexcept
    {
        if constexpr( std::same_as<E, DocumentChanged> )
        {
            return 0;
        }
        else if constexpr( std::same_as<E, RegenStateChanged> )
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }

    [[nodiscard]] Subscription add( std::size_t kind, std::function<void( const void* )> call );
    void                       dispatch( std::size_t kind, const void* event ) const;
    [[nodiscard]] std::size_t  countActive( std::size_t kind ) const;

    mutable std::mutex                                                          m_mutex;
    mutable std::array<std::vector<std::shared_ptr<detail::EventSlot>>, kKinds> m_slots;
};

} // namespace app::core
