// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/EventBusTests.cpp
/// @brief  Unit tests for the event bus (T-01, A-03, A-07).

#include "core/EventBus.h"

#include <chrono>
#include <gtest/gtest.h>

#include <atomic>
#include <latch>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using app::core::DocumentChanged;
using app::core::EventBus;
using app::core::JobProgress;
using app::core::ObjectIdGenerator;
using app::core::RegenState;
using app::core::RegenStateChanged;
using app::core::Subscription;

TEST( EventBus, DeliversToHandlersOfTheType_InSubscriptionOrder )
{
    EventBus                 bus;
    ObjectIdGenerator        ids = ObjectIdGenerator::makeDeterministic( 7, 1'700'000'000'000 );
    std::vector<std::string> calls;

    const Subscription first = bus.subscribe<DocumentChanged>(
        [&]( const DocumentChanged& e )
        {
            calls.push_back( "first:" + e.documentId.toString() );
        } );
    const Subscription second = bus.subscribe<DocumentChanged>(
        [&]( const DocumentChanged& e )
        {
            calls.push_back( "second:" + std::to_string( e.changes.added.size() ) );
        } );
    const Subscription regen = bus.subscribe<RegenStateChanged>(
        [&]( const RegenStateChanged& e )
        {
            calls.push_back( e.state == RegenState::Error ? "regen:error" : "regen" );
        } );

    const app::core::ObjectId document = ids.next();

    bus.publish( DocumentChanged{ document, { { ids.next(), ids.next() }, {}, {} } } );
    bus.publish( RegenStateChanged{ ids.next(), RegenState::Error } );
    bus.publish( JobProgress{ 1, 0.5, "Meshing" } );

    EXPECT_EQ( calls, ( std::vector<std::string>{ "first:" + document.toString(), "second:2", "regen:error" } ) );
    EXPECT_EQ( bus.subscriberCount<DocumentChanged>(), 2U );
    EXPECT_EQ( bus.subscriberCount<JobProgress>(), 0U );
}

TEST( EventBus, StopsCalling_WhenSubscriptionIsReleased )
{
    EventBus bus;
    int      calls = 0;

    Subscription subscription = bus.subscribe<JobProgress>(
        [&]( const JobProgress& )
        {
            ++calls;
        } );

    bus.publish( JobProgress{} );

    Subscription moved = std::move( subscription );

    EXPECT_FALSE( subscription.isActive() ); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move): tested
    EXPECT_TRUE( moved.isActive() );

    bus.publish( JobProgress{} );
    moved.reset();
    moved.reset();
    bus.publish( JobProgress{} );

    {
        const Subscription scoped = bus.subscribe<JobProgress>(
            [&]( const JobProgress& )
            {
                calls += 10;
            } );

        bus.publish( JobProgress{} );
    }

    bus.publish( JobProgress{} );

    EXPECT_EQ( calls, 12 );
    EXPECT_FALSE( moved.isActive() );
    EXPECT_EQ( bus.subscriberCount<JobProgress>(), 0U );
}

TEST( EventBus, ReportsDispatching_OnlyWhileAHandlerRuns )
{
    EventBus            bus;
    std::optional<bool> inside;

    const Subscription subscription = bus.subscribe<JobProgress>(
        [&]( const JobProgress& )
        {
            inside = EventBus::isDispatching();
        } );

    EXPECT_FALSE( EventBus::isDispatching() );
    bus.publish( JobProgress{} );
    EXPECT_EQ( inside, std::optional<bool>( true ) );
    EXPECT_FALSE( EventBus::isDispatching() );
}

TEST( EventBus, AllowsSubscriptionChanges_FromInsideAHandler )
{
    EventBus     bus;
    int          selfCalls  = 0;
    int          laterCalls = 0;
    int          nested     = 0;
    Subscription self;
    Subscription later;

    self = bus.subscribe<JobProgress>(
        [&]( const JobProgress& )
        {
            ++selfCalls;
            self.reset();

            if( !later.isActive() )
            {
                later = bus.subscribe<JobProgress>(
                    [&]( const JobProgress& )
                    {
                        ++laterCalls;
                    } );
            }

            bus.publish( RegenStateChanged{} );
        } );

    const Subscription regen = bus.subscribe<RegenStateChanged>(
        [&]( const RegenStateChanged& )
        {
            ++nested;
        } );

    bus.publish( JobProgress{} );
    EXPECT_EQ( laterCalls, 0 );

    bus.publish( JobProgress{} );

    EXPECT_EQ( selfCalls, 1 );
    EXPECT_EQ( laterCalls, 1 );
    EXPECT_EQ( nested, 1 );
}

TEST( EventBus, WaitsForRunningHandler_WhenReleasedFromAnotherThread )
{
    EventBus          bus;
    std::latch        entered( 1 );
    std::latch        release( 1 );
    std::atomic<bool> handlerDone{ false };
    std::atomic<bool> resetDone{ false };

    Subscription subscription = bus.subscribe<JobProgress>(
        [&]( const JobProgress& )
        {
            entered.count_down();
            release.wait();
            handlerDone = true;
        } );

    std::thread publisher(
        [&]
        {
            bus.publish( JobProgress{} );
        } );

    entered.wait();

    std::thread releaser(
        [&]
        {
            subscription.reset();
            resetDone = handlerDone.load();
        } );

    std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

    EXPECT_FALSE( handlerDone.load() );
    release.count_down();
    publisher.join();
    releaser.join();

    EXPECT_TRUE( resetDone.load() );
    EXPECT_FALSE( subscription.isActive() );
}

TEST( EventBus, DeliversEveryEvent_WhenPublishedFromManyThreads )
{
    EventBus           bus;
    std::atomic<int>   received{ 0 };
    const Subscription subscription = bus.subscribe<JobProgress>(
        [&]( const JobProgress& )
        {
            ++received;
        } );

    std::vector<std::thread> threads;

    threads.reserve( 8 );

    for( int t = 0; t < 8; ++t )
    {
        threads.emplace_back(
            [&]
            {
                for( int i = 0; i < 1000; ++i )
                {
                    bus.publish( JobProgress{ static_cast<std::uint64_t>( i ), 0.0, {} } );
                }
            } );
    }

    for( std::thread& thread : threads )
    {
        thread.join();
    }

    EXPECT_EQ( received.load(), 8000 );
}

TEST( EventBus, KeepsSubscriptionSafe_WhenTheBusIsDestroyedFirst )
{
    Subscription subscription;

    {
        EventBus bus;

        subscription = bus.subscribe<JobProgress>( []( const JobProgress& ) {} );
        EXPECT_TRUE( subscription.isActive() );
    }

    EXPECT_FALSE( subscription.isActive() );
    subscription.reset();
}
