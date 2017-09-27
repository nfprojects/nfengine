/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Async thread pool classes declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/ConditionVariable.hpp"
#include "../Containers/SharedPtr.hpp"
#include "../Containers/DynArray.hpp"

#include <functional>
#include <inttypes.h>
#include <thread>
#include <atomic>
#include <queue>

namespace NFE {
namespace Common {


struct AsyncTaskContext
{
    const AsyncTask& task;
    uint32 threadId;
};

/**
 * Function object representing a task.
 * @param thread   Thread id
 */
using AsyncFuncCallback = std::function<void(const AsyncTaskContext&)>;

class AsyncThreadPool;
class IAsyncDependency;
class AsyncTask;

using AsyncDependencyPtr = SharedPtr<IAsyncDependency>;
using AsyncDependencyWeakPtr = WeakPtr<IAsyncDependency>;
using AsyncTaskPtr = SharedPtr<AsyncTask>;


/**
 * Generic task dependency.
 */
class IAsyncDependency
{
public:
    explicit IAsyncDependency(AsyncThreadPool& pool);
    virtual ~IAsyncDependency() = default;

    // Return parent tread pool.
    NFE_INLINE AsyncThreadPool& GetPool() const { return mPool; }

    // Check if the dependency is fulfilled
    NFE_INLINE bool IsFulfilled() const { return mFullfilled; }

protected:
    /**
     * Called when dependency has been fulfilled.
     */
    virtual void OnDepenencyFulfilled();

private:
    AsyncThreadPool& mPool;

    // list of tasks that are dependent on this dependency
    DynArray<AsyncTaskPtr> mReverseDependencies;

    std::atomic<bool> mFullfilled;
};


struct AsyncTaskSetup
{
    AsyncTaskPtr parent;
    AsyncDependencyPtr dependency;
};

class NFE_ALIGN(64) AsyncTask : public IAsyncDependency
{
public:

    enum class State : uint8
    {
        Pending,    // created, not scheduled
        Sheduled,   // scheduled for execution (waiting for dependency)
        Enqueued,   // ready for execution, waiting in queue
        Executing,  // task is being executed at this moment
        Executed,   // executed
    };

    AsyncTask(AsyncThreadPool& pool, const AsyncFuncCallback& callback, const AsyncTaskSetup& setup);
    ~AsyncTask();

    // Get task state
    NFE_INLINE State GetState() const { return mState; }

    /**
     * Schedule a pending scheduled task for execution.
     * @remarks Calling this function second is forbidden.
     * @remarks This method is thread-safe.
     */
    void Schedule();
    void Enqueue();

    /**
     * Execute the task now. Should be called by the pool only.
     */
    void OnExecute();

    /**
     * Wait until the task is finished.
     * @remarks This can be called ONLY ON MAIN THREAD - do not abuse.
     */
    void Wait() const;

    void UpdateState(const State newState);

private:

    void OnFinished();

    AsyncTaskPtr mParent;                   // parent task
    AsyncDependencyPtr mDependency;

    const AsyncFuncCallback mCallback;

    std::atomic<uint32> mTasksLeft;
    std::atomic<State> mState;
};


/**
 * @class AsyncThreadPool
 * @brief Class enabling executing asynchronous functions in a simple thread pool.
 */
class NFCOMMON_API AsyncThreadPool final
{
    NFE_MAKE_NONCOPYABLE(AsyncThreadPool);
    NFE_MAKE_NONMOVEABLE(AsyncThreadPool);

public:
    AsyncThreadPool();
    ~AsyncThreadPool();

    std::enable_shared_from_this
    /**
     * Create a new task.
     * @remarks This function is thread-safe.
     * @remarks Created task IS NOT scheduled automatically - it must be done manually.
     * @param function  Task routine (can be null for creating synchronization point only).
     * @return          Task pointer.
     */
    AsyncTaskPtr CreateTask(const AsyncFuncCallback& function, const AsyncTaskSetup& setup = AsyncTaskSetup());

private:
    void EnqueueTask(const AsyncTaskPtr& task);

    void SchedulerCallback();

    // worker thread routine
    void WorkerThreadCallback();

    DynArray<std::thread> mWorkerThreads;

    ConditionVariable mFinishedTasksCV; // notifies about finished task
    Mutex mFinishedTaskMutex;

    ConditionVariable mTaskQueueCV; // notifies about a new task in the queue
    std::queue<AsyncTaskPtr> mTasksQueue;
    Mutex mTasksQueueMutex;  //< lock for "mTasksQueue" access

    std::atomic<bool> mStarted;
};

} // namespace Common
} // namespace NFE
