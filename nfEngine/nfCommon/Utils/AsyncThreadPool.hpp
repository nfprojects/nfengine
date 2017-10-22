/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Async thread pool classes declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/ConditionVariable.hpp"
#include "../Containers/HashMap.hpp"
#include "../Containers/DynArray.hpp"

#include <functional>
#include <inttypes.h>
#include <thread>
#include <atomic>
#include <queue>

namespace NFE {
namespace Common {

/**
 * Thread pool task unique identifier.
 */
typedef uint64_t AsyncFuncID;

/**
 * Function object representing a task.
 * @param instance Instance id of the whole task
 * @param thread   Thread id
 */
typedef std::function<void()> AsyncFuncCallback;

/**
 * @class Task
 * @brief Internal task structure.
 * @remarks Only @p AsyncThreadPool class can access it.
 */
class AsyncFunc final
{
    friend class AsyncThreadPool;

    AsyncFuncID ptr;
    AsyncFuncCallback mCallback;

public:
    AsyncFunc(AsyncFuncCallback callback);
};


/**
 * @class AsyncThreadPool
 * @brief Class enabling executing asynchronous functions in a simple thread pool.
 */
class NFCOMMON_API AsyncThreadPool final
{
    DynArray<std::thread> mWorkerThreads;

    ConditionVariable mTaskQueueTask;
    std::queue<AsyncFunc*> mTasksQueue;
    Mutex mTasksQueueMutex;  //< lock for "mTasksQueue" access

    std::atomic<bool> mStarted;
    std::atomic<AsyncFuncID> mLastTaskId;
    HashMap<AsyncFuncID, AsyncFunc*> mTasks;
    Mutex mTasksMutex;                 //< lock for "mTasks"
    ConditionVariable mTasksMutexCV;  //< condition variable used to notify about finished task

    // translate AsyncFuncID to Task object
    AsyncFunc* GetTask(const AsyncFuncID& taskID) const;

    void SchedulerCallback();

    // worker thread routine
    void WorkerThreadCallback();

public:
    AsyncThreadPool();
    ~AsyncThreadPool();

    /**
     * Create a new task.
     *
     * @remarks This function is thread-safe.
     * @param function  Task routine (can be null for creating synchronization point only).
     * @return          Task ID
     */
    AsyncFuncID Enqueue(AsyncFuncCallback function);

    /**
     * Check if a task is completed.
     */
    bool IsTaskFinished(const AsyncFuncID& taskID);

    /**
     * Waits for a task to finish.
     */
    void WaitForTask(const AsyncFuncID& taskID);

    /**
     * Waits for all tasks in the pool to finish.
     */
    void WaitForAllTasks();
};

} // namespace Common
} // namespace NFE
