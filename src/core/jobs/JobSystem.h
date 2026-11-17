// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/jobs/JobSystem.h
/// @brief  Thread pool running jobs with dependencies, progress reporting and cooperative cancellation.
/// @req    A-10, A-07

#pragma once

#include "core/jobs/CancellationToken.h"
#include "core/jobs/Progress.h"
#include "core/Result.h"

#include <chrono>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace app::core
{
class EventBus;
} // namespace app::core

namespace app::core::jobs
{

/// @brief Where a job is in its life. Succeeded, Failed and Cancelled are final.
enum class JobStatus : std::uint8_t
{
    Pending,   ///< Waiting for its dependencies or for a free worker.
    Running,   ///< Running on a worker thread.
    Succeeded, ///< The function returned success.
    Failed,    ///< The function returned an error, or threw.
    Cancelled, ///< Cancelled before or while running, or a dependency did not succeed.
};

/// @brief  Stable lower-case name of a status, for example "running".
/// @param  status  Status.
/// @return The name.
[[nodiscard]] std::string_view toString( JobStatus status ) noexcept;

/// @brief What a running job can see: its cancellation token and its progress.
class JobContext
{
public:
    /// @brief Context of one run.
    /// @param id        The job's ID.
    /// @param token     Cancellation of the job.
    /// @param progress  Progress of the job.
    JobContext( std::uint64_t id, CancellationToken token, Progress& progress ) noexcept;

    /// @brief  The job's ID, as in JobProgress events.
    /// @return The ID.
    [[nodiscard]] std::uint64_t jobId() const noexcept;

    /// @brief  Cancellation of the job; poll it and return ErrorCode::Cancelled soon after it is requested.
    /// @return The token.
    [[nodiscard]] const CancellationToken& token() const noexcept;

    /// @brief  Whether cancellation of the job has been requested.
    /// @return token().isCancellationRequested().
    [[nodiscard]] bool isCancellationRequested() const noexcept;

    /// @brief Report progress (see Progress::report()).
    /// @param fraction  Completed fraction.
    /// @param message   What the job is doing; empty keeps the current message.
    void reportProgress( double fraction, std::string_view message = {} );

private:
    std::uint64_t     m_id;
    CancellationToken m_token;
    Progress&         m_progress;
};

/// @brief The work of a job. It runs on a worker thread, must not touch documents (it works on snapshots, A-10), and
///        returns its outcome; results are handed back through captured state and applied by commands.
using JobFunction = std::function<Result<>( JobContext& )>;

namespace detail
{
/// @cond INTERNAL
struct JobState;
/// @endcond
} // namespace detail

/// @brief Shared handle to a submitted job; copies refer to the same job, and the job runs whether or not handles
///        remain. A default handle refers to no job. Thread-safe.
class JobHandle
{
public:
    /// @brief A handle to no job.
    JobHandle() noexcept = default;

    /// @brief  Whether the handle refers to a job.
    /// @return False for a default handle.
    [[nodiscard]] bool isValid() const noexcept;

    /// @brief  The job's ID, unique within its job system and never 0.
    /// @return The ID, or 0 for a default handle.
    [[nodiscard]] std::uint64_t id() const noexcept;

    /// @brief  The name given when it was submitted.
    /// @return The name; empty for a default handle.
    [[nodiscard]] std::string name() const;

    /// @brief  The current status.
    /// @return The status; Cancelled for a default handle.
    [[nodiscard]] JobStatus status() const;

    /// @brief  Whether the status is final.
    /// @return True for Succeeded, Failed, Cancelled and for a default handle.
    [[nodiscard]] bool isFinished() const;

    /// @brief  The current progress.
    /// @return Fraction and message.
    [[nodiscard]] ProgressSnapshot progress() const;

    /// @brief Request cancellation. A pending job is cancelled at once, without running; a running job sees its token
    ///        cancelled. Dependants of a job that does not succeed are cancelled.
    void cancel() const;

    /// @brief  Block until the job is finished. Never call it from inside a job (use dependencies instead), nor from
    ///         the UI thread (A-10).
    /// @return Success, the job's error, ErrorCode::Cancelled, or ErrorCode::InvalidArgument for a default handle.
    [[nodiscard]] Result<> wait() const;

    /// @brief  Block until the job is finished or the timeout passes.
    /// @param  timeout  Longest wait.
    /// @return True if the job is finished.
    [[nodiscard]] bool waitFor( std::chrono::milliseconds timeout ) const;

private:
    friend class JobSystem;

    explicit JobHandle( std::shared_ptr<detail::JobState> state ) noexcept;

    std::shared_ptr<detail::JobState> m_state;
};

/// @brief Options for submitting a job.
struct JobOptions
{
    std::string            name;         ///< Shown with its progress, for example "Regenerate Part1".
    std::vector<JobHandle> dependencies; ///< Jobs of the same system that must succeed before this one runs.
};

/// @brief Thread pool with a dependency graph. Jobs run in submission order once their dependencies have succeeded;
///        independent jobs run in parallel. Progress is published as JobProgress events when an event bus is given
///        (from worker threads; the bus is thread-safe). Thread-safe.
class JobSystem
{
public:
    /// @brief Start the worker threads.
    /// @param workerCount  Number of worker threads; 0 is treated as 1.
    /// @param events       Bus for JobProgress events; may be null. Must outlive the job system.
    explicit JobSystem( std::size_t workerCount = defaultWorkerCount(), EventBus* events = nullptr );

    JobSystem( const JobSystem& )            = delete;
    JobSystem& operator=( const JobSystem& ) = delete;
    JobSystem( JobSystem&& )                 = delete;
    JobSystem& operator=( JobSystem&& )      = delete;

    /// @brief Cancel every unfinished job, wait for running jobs to return and stop the workers.
    ~JobSystem();

    /// @brief  Submit a job.
    /// @param  function  The work.
    /// @param  options   Name and dependencies.
    /// @return A handle to the job. A job with an empty function, or with a dependency that is a default handle or
    ///         belongs to another job system, fails at once with ErrorCode::InvalidArgument.
    [[nodiscard]] JobHandle submit( JobFunction function, JobOptions options = {} );

    /// @brief Cancel every unfinished job.
    void cancelAll();

    /// @brief  Number of worker threads.
    /// @return The count.
    [[nodiscard]] std::size_t workerCount() const noexcept;

    /// @brief  One fewer than the number of hardware threads (leaving one for the UI), and at least 1.
    /// @return The count.
    [[nodiscard]] static std::size_t defaultWorkerCount() noexcept;

    /// @cond INTERNAL
    struct Scheduler;
    /// @endcond

private:
    std::shared_ptr<Scheduler> m_scheduler;
};

} // namespace app::core::jobs
