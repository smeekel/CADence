// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/JobSystemTests.cpp
/// @brief  Unit tests for the job system, cancellation tokens and progress (T-01, A-10).

#include "core/EventBus.h"
#include "core/jobs/CancellationToken.h"
#include "core/jobs/JobSystem.h"
#include "core/jobs/Progress.h"

#include <chrono>
#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

using app::ErrorCode;
using app::fail;
using app::ok;
using app::Result;
using app::core::EventBus;
using app::core::JobProgress;
using app::core::Subscription;
using app::core::jobs::CancellationSource;
using app::core::jobs::CancellationToken;
using app::core::jobs::JobContext;
using app::core::jobs::JobHandle;
using app::core::jobs::JobStatus;
using app::core::jobs::JobSystem;
using app::core::jobs::Progress;
using app::core::jobs::ProgressSnapshot;

namespace
{

using namespace std::chrono_literals;

constexpr auto kPatience = 5s; // generous bound so a slow machine cannot hang a test

// Block until cancellation is requested, without polling.
Result<> waitForCancellation( JobContext& context )
{
    std::mutex                  mutex;
    std::condition_variable_any condition;
    std::unique_lock            lock( mutex );

    condition.wait( lock,
                    context.token().stopToken(),
                    []
                    {
                        return false;
                    } );
    return fail( ErrorCode::Cancelled, "Cancelled while waiting." );
}

// A gate that jobs wait on until the test opens it (or the patience runs out).
class Gate
{
public:
    void open()
    {
        {
            const std::scoped_lock lock( m_mutex );
            m_open = true;
        }
        m_condition.notify_all();
    }

    [[nodiscard]] bool waitOpen()
    {
        std::unique_lock lock( m_mutex );

        return m_condition.wait_for( lock,
                                     kPatience,
                                     [this]
                                     {
                                         return m_open;
                                     } );
    }

private:
    std::mutex              m_mutex;
    std::condition_variable m_condition;
    bool                    m_open = false;
};

[[nodiscard]] bool waitUntilRunning( const JobHandle& job )
{
    const auto deadline = std::chrono::steady_clock::now() + kPatience;

    while( job.status() == JobStatus::Pending )
    {
        if( std::chrono::steady_clock::now() > deadline )
        {
            return false;
        }

        std::this_thread::yield();
    }

    return true;
}

} // namespace

TEST( CancellationToken, IsNeverCancelled_WhenDefaultConstructed )
{
    const CancellationToken token;

    EXPECT_FALSE( token.isCancellationRequested() );
    EXPECT_FALSE( token.canBeCancelled() );
}

TEST( CancellationToken, SeesTheRequest_WhenItsSourceCancels )
{
    CancellationSource       source;
    const CancellationToken  token  = source.token();
    bool                     called = false;
    const std::stop_callback callback( token.stopToken(),
                                       [&called]
                                       {
                                           called = true;
                                       } );

    EXPECT_TRUE( token.canBeCancelled() );
    EXPECT_FALSE( token.isCancellationRequested() );

    EXPECT_TRUE( source.requestCancellation() );
    EXPECT_FALSE( source.requestCancellation() );

    EXPECT_TRUE( token.isCancellationRequested() );
    EXPECT_TRUE( source.token().isCancellationRequested() );
    EXPECT_TRUE( called );
}

TEST( Progress, NeverDecreases_AndStaysWithinZeroAndOne )
{
    Progress progress;

    progress.report( 0.4, "Meshing" );
    progress.report( 0.2 );
    EXPECT_DOUBLE_EQ( progress.snapshot().fraction, 0.4 );
    EXPECT_EQ( progress.snapshot().message, "Meshing" );

    progress.report( std::numeric_limits<double>::quiet_NaN(), "Writing" );
    EXPECT_DOUBLE_EQ( progress.snapshot().fraction, 0.4 );
    EXPECT_EQ( progress.snapshot().message, "Writing" );

    progress.report( 7.0 );
    EXPECT_DOUBLE_EQ( progress.snapshot().fraction, 1.0 );

    Progress other;
    other.report( -3.0 );
    EXPECT_DOUBLE_EQ( other.snapshot().fraction, 0.0 );
}

TEST( Progress, PublishesIncreasingFractions_WhenReportedFromManyThreads )
{
    std::mutex          mutex;
    std::vector<double> published;
    Progress            progress(
        [&]( const ProgressSnapshot& snapshot )
        {
            const std::scoped_lock lock( mutex );
            published.push_back( snapshot.fraction );
        } );
    {
        std::vector<std::jthread> threads;
        threads.reserve( 4 );
        for( int thread = 0; thread < 4; ++thread )
        {
            threads.emplace_back(
                [&progress, thread]
                {
                    for( int step = 0; step <= 1'000; ++step )
                    {
                        progress.report( ( step * 4 + thread ) / 4'003.0 );
                    }
                } );
        }
    }

    ASSERT_FALSE( published.empty() );
    EXPECT_TRUE( std::ranges::is_sorted( published ) );
    EXPECT_DOUBLE_EQ( published.back(), 1.0 );
}

TEST( Progress, PublishesSmallStepsOnlyOnceTheyAddUp )
{
    int      calls = 0;
    Progress progress(
        [&calls]( const ProgressSnapshot& )
        {
            ++calls;
        } );

    for( int step = 0; step <= 100'000; ++step )
    {
        progress.report( step / 100'000.0 );
    }

    EXPECT_LE( calls, static_cast<int>( 1.0 / Progress::kPublishStep ) + 2 );
    EXPECT_GE( calls, 100 );
}

TEST( JobSystem, RunsAJob_AndReportsItsOutcome )
{
    JobSystem        jobs( 2 );
    std::atomic<int> value = 0;

    const JobHandle good = jobs.submit(
        [&value]( JobContext& ) -> Result<>
        {
            value = 42;
            return ok();
        },
        { .name = "Answer" } );
    const JobHandle bad = jobs.submit(
        []( JobContext& ) -> Result<>
        {
            return fail( ErrorCode::GeometryFailure, "The fillet failed." );
        } );

    EXPECT_TRUE( good.wait().has_value() );
    EXPECT_EQ( value, 42 );
    EXPECT_EQ( good.status(), JobStatus::Succeeded );
    EXPECT_EQ( good.name(), "Answer" );
    EXPECT_NE( good.id(), bad.id() );

    const Result<> failure = bad.wait();
    ASSERT_FALSE( failure.has_value() );
    EXPECT_EQ( failure.error().code, ErrorCode::GeometryFailure );
    EXPECT_EQ( bad.status(), JobStatus::Failed );
}

TEST( JobSystem, StopsARunningJob_Within50Milliseconds_WhenCancelled )
{
    JobSystem       jobs( 1 );
    const JobHandle job = jobs.submit( waitForCancellation );
    ASSERT_TRUE( waitUntilRunning( job ) );

    const auto start = std::chrono::steady_clock::now();
    job.cancel();
    const Result<> outcome = job.wait();
    const auto     elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_LT( elapsed, 50ms );
    ASSERT_FALSE( outcome.has_value() );
    EXPECT_EQ( outcome.error().code, ErrorCode::Cancelled );
    EXPECT_EQ( job.status(), JobStatus::Cancelled );
}

TEST( JobSystem, CancelsAPendingJob_WithoutRunningIt )
{
    JobSystem         jobs( 1 );
    Gate              gate;
    std::atomic<bool> ran = false;

    const JobHandle blocker = jobs.submit(
        [&gate]( JobContext& ) -> Result<>
        {
            return gate.waitOpen() ? ok() : fail( ErrorCode::Internal, "Timeout." );
        } );
    const JobHandle queued = jobs.submit(
        [&ran]( JobContext& ) -> Result<>
        {
            ran = true;
            return ok();
        },
        { .name = "Queued" } );

    queued.cancel();
    EXPECT_EQ( queued.status(), JobStatus::Cancelled );

    gate.open();
    EXPECT_TRUE( blocker.wait().has_value() );

    const Result<> outcome = queued.wait();
    ASSERT_FALSE( outcome.has_value() );
    EXPECT_EQ( outcome.error().message, "'Queued' was cancelled." );
    EXPECT_FALSE( ran );
}

TEST( JobSystem, RunsDependentJobsAfterTheirDependencies )
{
    JobSystem                jobs( 4 );
    std::mutex               mutex;
    std::vector<std::string> order;
    const auto               record = [&]( const char* name )
    {
        return [&order, &mutex, name]( JobContext& ) -> Result<>
        {
            std::this_thread::sleep_for( 1ms );
            const std::scoped_lock lock( mutex );
            order.push_back( name );
            return ok();
        };
    };

    // Diamond a -> (b, c) -> d, then e after d.
    const JobHandle a = jobs.submit( record( "a" ) );
    const JobHandle b = jobs.submit( record( "b" ), { .dependencies = { a } } );
    const JobHandle c = jobs.submit( record( "c" ), { .dependencies = { a } } );
    const JobHandle d = jobs.submit( record( "d" ), { .dependencies = { b, c } } );
    const JobHandle e = jobs.submit( record( "e" ), { .dependencies = { d } } );

    ASSERT_TRUE( e.wait().has_value() );

    const auto position = [&order]( const std::string& name )
    {
        return std::ranges::find( order, name ) - order.begin();
    };

    ASSERT_EQ( order.size(), 5U );
    EXPECT_LT( position( "a" ), position( "b" ) );
    EXPECT_LT( position( "a" ), position( "c" ) );
    EXPECT_LT( position( "b" ), position( "d" ) );
    EXPECT_LT( position( "c" ), position( "d" ) );
    EXPECT_LT( position( "d" ), position( "e" ) );
}

TEST( JobSystem, RunsIndependentJobsInParallel )
{
    JobSystem        jobs( 2 );
    std::atomic<int> arrived = 0;
    const auto       meet    = [&arrived]( JobContext& ) -> Result<>
    {
        ++arrived;
        const auto deadline = std::chrono::steady_clock::now() + kPatience;

        while( arrived < 2 )
        {
            if( std::chrono::steady_clock::now() > deadline )
            {
                return fail( ErrorCode::Internal, "The other job never ran at the same time." );
            }

            std::this_thread::yield();
        }

        return ok();
    };

    const JobHandle first  = jobs.submit( meet );
    const JobHandle second = jobs.submit( meet );

    EXPECT_TRUE( first.wait().has_value() );
    EXPECT_TRUE( second.wait().has_value() );
}

TEST( JobSystem, CancelsDependants_WhenADependencyFails )
{
    JobSystem         jobs( 2 );
    std::atomic<bool> ran  = false;
    const auto        mark = [&ran]( JobContext& ) -> Result<>
    {
        ran = true;
        return ok();
    };

    const JobHandle source = jobs.submit(
        []( JobContext& ) -> Result<>
        {
            return fail( ErrorCode::IoError, "Disk." );
        },
        { .name = "Read" } );
    const JobHandle middle = jobs.submit( mark, { .name = "Mesh", .dependencies = { source } } );
    const JobHandle last   = jobs.submit( mark, { .name = "Render", .dependencies = { middle } } );

    const Result<> outcome = last.wait();
    ASSERT_FALSE( outcome.has_value() );
    EXPECT_EQ( outcome.error().code, ErrorCode::Cancelled );
    EXPECT_EQ( middle.wait().error().message, "'Mesh' did not run because 'Read' failed." );
    EXPECT_EQ( outcome.error().message, "'Render' did not run because 'Mesh' was cancelled." );
    EXPECT_FALSE( ran );

    const JobHandle late = jobs.submit( mark, { .name = "Late", .dependencies = { source } } );
    EXPECT_EQ( late.status(), JobStatus::Cancelled );
    EXPECT_FALSE( ran );
}

TEST( JobSystem, FailsAJob_WhenItThrows_AndKeepsRunningOthers )
{
    JobSystem       jobs( 1 );
    const JobHandle thrower = jobs.submit(
        []( JobContext& ) -> Result<>
        {
            throw std::runtime_error( "boom" );
        },
        { .name = "Thrower" } );
    const JobHandle next = jobs.submit(
        []( JobContext& ) -> Result<>
        {
            return ok();
        } );

    const Result<> outcome = thrower.wait();
    ASSERT_FALSE( outcome.has_value() );
    EXPECT_EQ( outcome.error().code, ErrorCode::Internal );
    EXPECT_EQ( outcome.error().detail, "boom" );
    EXPECT_TRUE( next.wait().has_value() );
}

TEST( JobSystem, RejectsAJob_WithoutAFunctionOrWithAForeignDependency )
{
    JobSystem jobs( 1 );
    JobSystem other( 1 );

    const JobHandle foreign = other.submit(
        []( JobContext& ) -> Result<>
        {
            return ok();
        } );
    const JobHandle bad = jobs.submit(
        []( JobContext& ) -> Result<>
        {
            return ok();
        },
        { .dependencies = { foreign } } );
    const JobHandle empty = jobs.submit( {} );
    const JobHandle none  = jobs.submit(
        []( JobContext& ) -> Result<>
        {
            return ok();
        },
        { .dependencies = { JobHandle() } } );

    for( const JobHandle& job : { bad, empty, none } )
    {
        const Result<> outcome = job.wait();
        ASSERT_FALSE( outcome.has_value() );
        EXPECT_EQ( outcome.error().code, ErrorCode::InvalidArgument );
    }

    EXPECT_FALSE( JobHandle().wait().has_value() );
    EXPECT_TRUE( JobHandle().isFinished() );
}

TEST( JobSystem, PublishesProgressEvents_ForTheJob )
{
    EventBus            bus;
    std::mutex          mutex;
    std::vector<double> fractions;
    std::string         lastMessage;
    std::uint64_t       seenId = 0;

    const Subscription subscription = bus.subscribe<JobProgress>(
        [&]( const JobProgress& event )
        {
            const std::scoped_lock lock( mutex );
            fractions.push_back( event.fraction );
            lastMessage = event.message;
            seenId      = event.jobId;
        } );

    JobSystem       jobs( 1, &bus );
    const JobHandle job = jobs.submit(
        []( JobContext& context ) -> Result<>
        {
            context.reportProgress( 0.25, "Reading" );
            context.reportProgress( 0.1 );
            context.reportProgress( 0.75, "Meshing" );
            context.reportProgress( 1.0 );
            return ok();
        } );

    ASSERT_TRUE( job.wait().has_value() );

    const std::scoped_lock lock( mutex );
    EXPECT_EQ( fractions, ( std::vector<double>{ 0.25, 0.75, 1.0 } ) );
    EXPECT_EQ( lastMessage, "Meshing" );
    EXPECT_EQ( seenId, job.id() );
    EXPECT_DOUBLE_EQ( job.progress().fraction, 1.0 );
}

TEST( JobSystem, CancelsUnfinishedJobs_WhenDestroyed )
{
    JobHandle running;
    JobHandle pending;
    {
        JobSystem jobs( 1 );
        running = jobs.submit( waitForCancellation );
        pending = jobs.submit(
            []( JobContext& ) -> Result<>
            {
                return ok();
            },
            { .dependencies = { running } } );
        ASSERT_TRUE( waitUntilRunning( running ) );
    }

    EXPECT_EQ( running.status(), JobStatus::Cancelled );
    EXPECT_EQ( pending.status(), JobStatus::Cancelled );

    running.cancel();
    EXPECT_EQ( running.status(), JobStatus::Cancelled );
}
