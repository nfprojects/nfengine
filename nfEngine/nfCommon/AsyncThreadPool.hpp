/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Async thread pool classes declarations.
 */

#pragma once

#include "nfCommon.hpp"

#include <condition_variable>
#include <functional>
#include <inttypes.h>
#include <atomic>
#include <queue>
#include <map>

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
    typedef std::unique_lock<std::mutex> Lock;

    std::vector<std::thread> mWorkerThreads;

    std::condition_variable mTaskQueueTask;
    std::queue<AsyncFunc*> mTasksQueue;
    std::mutex mTasksQueueMutex;  //< lock for "mTasksQueue" access

    std::atomic<bool> mStarted;
    std::atomic<AsyncFuncID> mLastTaskId;
    std::map<AsyncFuncID, AsyncFunc*> mTasks;
    std::mutex mTasksMutex;                 //< lock for "mTasks"
    std::condition_variable mTasksMutexCV;  //< condition variable used to notify about finished task

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
