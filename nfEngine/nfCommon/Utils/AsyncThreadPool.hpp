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
    virtual ~IAsyncDependency() = default;

protected:
    /**
     * Called when dependency has been fulfilled.
     */
    void OnDepenencyFulfulled();

private:
    // list of tasks that are dependent on this dependency
    DynArray<AsyncTaskPtr> mReverseDependencies;
};


class NFE_ALIGN(64) AsyncTask : public IAsyncDependency
{
public:

    enum class State : uint8
    {
        Pending,    // created, not scheduled
        Sheduled,   // scheduled for execution (waiting in queue or for dependency)
        Executing,  // task is being executed at this moment
        Executed,   // executed
    };

    explicit AsyncTask(AsyncThreadPool& pool, const AsyncFuncCallback& callback);

    /**
     * Return parent pool.
     */
    NFE_INLINE AsyncThreadPool& GetPool() const { return mPool; }

    /**
     * Add a task dependency.
     * @remarks This must be called only when the task has not been scheduled for execution.
     * @remarks This method is thread-safe.
     */
    // TODO consider merging with AsyncThreadPool::CreateTask
    void AddDependency(const AsyncDependencyPtr& dep);
    void AddDependencies(const ArrayView<const AsyncDependencyPtr> deps);

    /**
     * Schedule a pending scheduled task for execution.
     * @remarks Calling this function second time has no effect.
     * @remarks This method is thread-safe.
     */
    void Schedule();

    /**
     * Execute the task now. Should be called by the pool only.
     */
    void Execute();

    /**
     * Wait until the task is finished.
     * @remarks This can be called ONLY ON MAIN THREAD - do not abuse.
     */
    void Wait() const;

private:

    AsyncThreadPool& mPool;

    // list of dependencies
    DynArray<AsyncDependencyWeakPtr> mDependencies;

    const AsyncFuncCallback mCallback;

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

    /**
     * Create a new task.
     * @remarks This function is thread-safe.
     * @remarks Created task IS NOT scheduled automatically - it must be done manually.
     * @param function  Task routine (can be null for creating synchronization point only).
     * @return          Task pointer.
     */
    AsyncTaskPtr CreateTask(const AsyncFuncCallback& function);

    /**
     * Waits for all tasks in the pool to finish.
     */
    void WaitForAllTasks();

private:
    void SchedulerCallback();

    // worker thread routine
    void WorkerThreadCallback();

    DynArray<std::thread> mWorkerThreads;

    ConditionVariable mTaskQueueTask;
    std::queue<AsyncTaskPtr> mTasksQueue;
    RWLock mTasksQueueMutex;  //< lock for "mTasksQueue" access

    std::atomic<bool> mStarted;
    RWLock mTasksMutex;                 //< lock for "mTasks"
    ConditionVariable mTasksMutexCV;  //< condition variable used to notify about finished task
};

} // namespace Common
} // namespace NFE
