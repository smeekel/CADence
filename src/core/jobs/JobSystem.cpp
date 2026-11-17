// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/jobs/JobSystem.cpp
/// @brief  Worker threads, the ready queue and the dependency bookkeeping of the job system.

#include "core/jobs/JobSystem.h"

#include "core/EventBus.h"

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <exception>
#include <format>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <utility>

namespace app::core::jobs
{

/// @cond INTERNAL
namespace detail
{

// Lock order: the scheduler's mutex before a job's mutex. Every change of status happens with both held, so a status
// read under either lock is stable while the scheduler's mutex is held.
struct JobState
{
    JobState( std::uint64_t jobId, std::string jobName, Progress::Listener listener )
        : id( jobId )
        , name( std::move( jobName ) )
        , progress( std::move( listener ) )
    {
    }

    const std::uint64_t                 id;
    const std::string                   name;
    std::weak_ptr<JobSystem::Scheduler> scheduler;
    CancellationSource                  cancellation;
    Progress                            progress;

    // Guarded by the scheduler's mutex.
    JobFunction                            function;
    std::size_t                            waitingFor = 0;
    std::vector<std::shared_ptr<JobState>> dependents;

    // Guarded by `mutex`.
    mutable std::mutex      mutex;
    std::condition_variable finished;
    JobStatus               status = JobStatus::Pending;
    std::optional<Error>    error;
};

} // namespace detail

struct JobSystem::Scheduler
{
    EventBus*                                                            events = nullptr;
    std::mutex                                                           mutex;
    std::condition_variable                                              work;
    std::deque<std::shared_ptr<detail::JobState>>                        ready;
    std::unordered_map<std::uint64_t, std::shared_ptr<detail::JobState>> unfinished;
    std::uint64_t                                                        nextId   = 1;
    bool                                                                 stopping = false;
    std::vector<std::jthread>                                            workers;
};

/// @endcond

namespace
{

using detail::JobState;
using Scheduler = JobSystem::Scheduler;

[[nodiscard]] bool isFinal( JobStatus status ) noexcept
{
    return status == JobStatus::Succeeded || status == JobStatus::Failed || status == JobStatus::Cancelled;
}

[[nodiscard]] std::string labelOf( const JobState& job )
{
    return job.name.empty() ? std::format( "Job {}", job.id ) : std::format( "'{}'", job.name );
}

[[nodiscard]] JobStatus statusOf( const JobState& job )
{
    const std::scoped_lock lock( job.mutex );

    return job.status;
}

// Finish a job and, transitively, cancel the dependants of a job that did not succeed or queue those whose last
// dependency succeeded. Requires the scheduler's mutex. Functions of jobs that will not run are moved to `garbage` so
// their captures are destroyed after the mutex is released.
void finishLocked( Scheduler&                       scheduler,
                   const std::shared_ptr<JobState>& job,
                   JobStatus                        status,
                   std::optional<Error>             error,
                   std::vector<JobFunction>&        garbage )
{
    struct Finish
    {
        std::shared_ptr<JobState> job;
        JobStatus                 status;
        std::optional<Error>      error;
    };

    std::vector<Finish> stack;
    stack.push_back( { job, status, std::move( error ) } );

    while( !stack.empty() )
    {
        Finish    item  = std::move( stack.back() );
        JobState& state = *item.job;
        stack.pop_back();
        {
            const std::scoped_lock lock( state.mutex );

            if( isFinal( state.status ) )
            {
                continue;
            }

            state.status = item.status;
            state.error  = std::move( item.error );
        }
        state.finished.notify_all();

        if( state.function )
        {
            garbage.push_back( std::move( state.function ) );
            state.function = nullptr;
        }

        scheduler.unfinished.erase( state.id );

        for( const std::shared_ptr<JobState>& dependent : state.dependents )
        {
            if( item.status == JobStatus::Succeeded )
            {
                if( --dependent->waitingFor == 0 )
                {
                    scheduler.ready.push_back( dependent );
                    scheduler.work.notify_one();
                }

                continue;
            }

            const char* const why = item.status == JobStatus::Failed ? "failed" : "was cancelled";

            stack.push_back(
                { dependent,
                  JobStatus::Cancelled,
                  fail( ErrorCode::Cancelled,
                        std::format( "{} did not run because {} {}.", labelOf( *dependent ), labelOf( state ), why ) )
                      .error() } );
        }

        state.dependents.clear();
    }
}

// Request cancellation of a job and finish it at once if it has not started.
void cancelLocked( Scheduler& scheduler, const std::shared_ptr<JobState>& job, std::vector<JobFunction>& garbage )
{
    job->cancellation.requestCancellation();

    if( statusOf( *job ) == JobStatus::Pending )
    {
        finishLocked( scheduler,
                      job,
                      JobStatus::Cancelled,
                      fail( ErrorCode::Cancelled, std::format( "{} was cancelled.", labelOf( *job ) ) ).error(),
                      garbage );
    }
}

void cancelAllLocked( Scheduler& scheduler, std::vector<JobFunction>& garbage )
{
    std::vector<std::shared_ptr<JobState>> jobs;
    jobs.reserve( scheduler.unfinished.size() );

    for( const auto& entry : scheduler.unfinished )
    {
        jobs.push_back( entry.second );
    }

    std::ranges::sort( jobs, {}, &JobState::id );

    for( const std::shared_ptr<JobState>& job : jobs )
    {
        cancelLocked( scheduler, job, garbage );
    }
}

// Run a job's function; an exception is turned into ErrorCode::Internal so that it cannot end the worker thread.
[[nodiscard]] Result<> runGuarded( const JobFunction& function, JobState& job )
{
    try
    {
        JobContext context( job.id, job.cancellation.token(), job.progress );

        return function( context );
    }
    catch( const std::exception& exception )
    {
        return fail( ErrorCode::Internal,
                     std::format( "{} stopped with an unexpected error.", labelOf( job ) ),
                     exception.what() );
    }
    catch( ... )
    {
        return fail( ErrorCode::Internal,
                     std::format( "{} stopped with an unexpected error.", labelOf( job ) ),
                     "unknown exception" );
    }
}

void runWorker( Scheduler& scheduler )
{
    std::unique_lock lock( scheduler.mutex );

    while( true )
    {
        scheduler.work.wait( lock,
                             [&scheduler]
                             {
                                 return scheduler.stopping || !scheduler.ready.empty();
                             } );

        if( scheduler.ready.empty() )
        {
            return;
        }

        const std::shared_ptr<JobState> job = std::move( scheduler.ready.front() );
        scheduler.ready.pop_front();
        {
            const std::scoped_lock jobLock( job->mutex );

            if( job->status != JobStatus::Pending )
            {
                continue;
            }

            job->status = JobStatus::Running;
        }

        JobFunction function = std::move( job->function );
        job->function        = nullptr;
        lock.unlock();

        Result<> outcome = runGuarded( function, *job );
        function         = nullptr;

        JobStatus            status = JobStatus::Succeeded;
        std::optional<Error> error;

        if( !outcome )
        {
            status = outcome.error().code == ErrorCode::Cancelled ? JobStatus::Cancelled : JobStatus::Failed;
            error  = std::move( outcome.error() );
        }

        std::vector<JobFunction> garbage;

        lock.lock();
        finishLocked( scheduler, job, status, std::move( error ), garbage );
        lock.unlock();
        garbage.clear();
        lock.lock();
    }
}

} // namespace

std::string_view toString( JobStatus status ) noexcept
{
    switch( status )
    {
        case JobStatus::Pending:
            return "pending";
        case JobStatus::Running:
            return "running";
        case JobStatus::Succeeded:
            return "succeeded";
        case JobStatus::Failed:
            return "failed";
        case JobStatus::Cancelled:
            return "cancelled";
    }

    return "unknown";
}

JobContext::JobContext( std::uint64_t id, CancellationToken token, Progress& progress ) noexcept
    : m_id( id )
    , m_token( std::move( token ) )
    , m_progress( progress )
{
}

std::uint64_t JobContext::jobId() const noexcept
{
    return m_id;
}

const CancellationToken& JobContext::token() const noexcept
{
    return m_token;
}

bool JobContext::isCancellationRequested() const noexcept
{
    return m_token.isCancellationRequested();
}

void JobContext::reportProgress( double fraction, std::string_view message )
{
    m_progress.report( fraction, message );
}

JobHandle::JobHandle( std::shared_ptr<detail::JobState> state ) noexcept
    : m_state( std::move( state ) )
{
}

bool JobHandle::isValid() const noexcept
{
    return m_state != nullptr;
}

std::uint64_t JobHandle::id() const noexcept
{
    return m_state ? m_state->id : 0;
}

std::string JobHandle::name() const
{
    return m_state ? m_state->name : std::string();
}

JobStatus JobHandle::status() const
{
    return m_state ? statusOf( *m_state ) : JobStatus::Cancelled;
}

bool JobHandle::isFinished() const
{
    return isFinal( status() );
}

ProgressSnapshot JobHandle::progress() const
{
    return m_state ? m_state->progress.snapshot() : ProgressSnapshot{};
}

void JobHandle::cancel() const
{
    if( !m_state )
    {
        return;
    }

    m_state->cancellation.requestCancellation();

    const std::shared_ptr<Scheduler> scheduler = m_state->scheduler.lock();

    if( !scheduler )
    {
        return;
    }

    std::vector<JobFunction> garbage;
    {
        const std::scoped_lock lock( scheduler->mutex );
        cancelLocked( *scheduler, m_state, garbage );
    }
}

Result<> JobHandle::wait() const
{
    if( !m_state )
    {
        return fail( ErrorCode::InvalidArgument, "The job handle refers to no job." );
    }

    std::unique_lock lock( m_state->mutex );
    m_state->finished.wait( lock,
                            [this]
                            {
                                return isFinal( m_state->status );
                            } );

    if( m_state->status == JobStatus::Succeeded )
    {
        return ok();
    }

    return std::unexpected( *m_state->error );
}

bool JobHandle::waitFor( std::chrono::milliseconds timeout ) const
{
    if( !m_state )
    {
        return true;
    }

    std::unique_lock lock( m_state->mutex );

    return m_state->finished.wait_for( lock,
                                       timeout,
                                       [this]
                                       {
                                           return isFinal( m_state->status );
                                       } );
}

JobSystem::JobSystem( std::size_t workerCount, EventBus* events )
    : m_scheduler( std::make_shared<Scheduler>() )
{
    m_scheduler->events = events;

    const std::size_t count = std::max<std::size_t>( workerCount, 1 );
    m_scheduler->workers.reserve( count );

    for( std::size_t index = 0; index < count; ++index )
    {
        m_scheduler->workers.emplace_back(
            [scheduler = m_scheduler.get()]
            {
                runWorker( *scheduler );
            } );
    }
}

JobSystem::~JobSystem()
{
    try
    {
        std::vector<JobFunction> garbage;
        {
            const std::scoped_lock lock( m_scheduler->mutex );
            m_scheduler->stopping = true;
            cancelAllLocked( *m_scheduler, garbage );
        }
        m_scheduler->work.notify_all();
        m_scheduler->workers.clear();
    }
    catch( ... )
    {
        // Only allocation can fail here; carrying on would leave jobs pending and their waiters blocked forever.
        std::terminate();
    }
}

JobHandle JobSystem::submit( JobFunction function, JobOptions options )
{
    Scheduler&                scheduler = *m_scheduler;
    std::vector<JobFunction>  garbage;
    std::shared_ptr<JobState> job;
    {
        const std::scoped_lock lock( scheduler.mutex );
        const std::uint64_t    id     = scheduler.nextId++;
        EventBus* const        events = scheduler.events;

        Progress::Listener listener;
        if( events != nullptr )
        {
            listener = [events, id]( const ProgressSnapshot& progress )
            {
                events->publish(
                    JobProgress{ .jobId = id, .fraction = progress.fraction, .message = progress.message } );
            };
        }

        job            = std::make_shared<JobState>( id, std::move( options.name ), std::move( listener ) );
        job->scheduler = m_scheduler;
        job->function  = std::move( function );
        scheduler.unfinished.emplace( id, job );

        const bool foreignDependency =
            std::ranges::any_of( options.dependencies,
                                 [this]( const JobHandle& dependency )
                                 {
                                     return !dependency.m_state || dependency.m_state->scheduler.lock() != m_scheduler;
                                 } );

        if( !job->function || foreignDependency )
        {
            const char* const what = !job->function ? "has no function" : "depends on a job of another job system";

            finishLocked( scheduler,
                          job,
                          JobStatus::Failed,
                          fail( ErrorCode::InvalidArgument, std::format( "{} {}.", labelOf( *job ), what ) ).error(),
                          garbage );
            return JobHandle( job );
        }

        const JobState* blocker = nullptr;

        for( const JobHandle& dependency : options.dependencies )
        {
            const JobStatus status = statusOf( *dependency.m_state );

            if( status == JobStatus::Succeeded )
            {
                continue;
            }

            if( isFinal( status ) )
            {
                blocker = dependency.m_state.get();
                break;
            }

            dependency.m_state->dependents.push_back( job );
            ++job->waitingFor;
        }

        if( blocker != nullptr )
        {
            const char* const why = statusOf( *blocker ) == JobStatus::Failed ? "failed" : "was cancelled";

            finishLocked(
                scheduler,
                job,
                JobStatus::Cancelled,
                fail( ErrorCode::Cancelled,
                      std::format( "{} did not run because {} {}.", labelOf( *job ), labelOf( *blocker ), why ) )
                    .error(),
                garbage );
        }
        else if( job->waitingFor == 0 )
        {
            scheduler.ready.push_back( job );
            scheduler.work.notify_one();
        }
    }

    return JobHandle( job );
}

void JobSystem::cancelAll()
{
    std::vector<JobFunction> garbage;
    {
        const std::scoped_lock lock( m_scheduler->mutex );
        cancelAllLocked( *m_scheduler, garbage );
    }
}

std::size_t JobSystem::workerCount() const noexcept
{
    return m_scheduler->workers.size();
}

std::size_t JobSystem::defaultWorkerCount() noexcept
{
    const unsigned int hardware = std::thread::hardware_concurrency();

    return hardware > 1 ? hardware - 1 : 1;
}

} // namespace app::core::jobs
