/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Thread pool classes declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "ThreadPoolTask.hpp"
#include "../System/ConditionVariable.hpp"
#include "../Containers/UniquePtr.hpp"
#include "../Containers/DynArray.hpp"

#include <inttypes.h>
#include <queue>
#include <thread>


namespace NFE {
namespace Common {


#define NFE_THREADPOOL_PRIORITIES 4
#define NFE_INVALID_TASK_ID (static_cast<::NFE::Common::TaskID>(-1))


/**
 * Structure describing task, used during Task creation.
 */
struct TaskDesc
{
    TaskFunction function;
    uint32 instancesNum;
    TaskID parent;
    TaskID dependency;
    uint8 priority;
    bool waitable;

    TaskDesc()
        : instancesNum(1)
        , parent(NFE_INVALID_TASK_ID)
        , dependency(NFE_INVALID_TASK_ID)
        , priority(0)
        , waitable(false)
    {}
};

/**
 * @class WorkerThread
 * @brief Executor thread
 */
class WorkerThread
{
    friend class ThreadPool;

    std::thread mThread;
    uint32 mId;    //< thread number
    bool mStarted; //< if set to false, exit the thread

    WorkerThread(const WorkerThread&) = delete;
    WorkerThread& operator = (const WorkerThread&) = delete;

public:
    WorkerThread(ThreadPool* pool, uint32 id);
    ~WorkerThread();
    WorkerThread(WorkerThread&&) = default;
    WorkerThread& operator = (WorkerThread&&) = default;
};

using WorkerThreadPtr = UniquePtr<WorkerThread>;

/**
 * @class ThreadPool
 * @brief Class enabling parallel tasks (user provided functions) execution.
 */
class NFCOMMON_API ThreadPool final
{
    friend class WorkerThread;

    /// Worker threads variables:
    uint32 mLastThreadId;
    DynArray<WorkerThreadPtr> mThreads;

    // queues for tasks with "Queued" state
    std::queue<TaskID> mTasksQueues[NFE_THREADPOOL_PRIORITIES];
    Mutex mTasksQueueMutex;                 //< lock for "mTasksQueue" access
    ConditionVariable mTaskQueueCV;         //< CV for notifying about a new task in the queue

    Mutex mFinishedTasksMutex;
    ConditionVariable mFinishedTasksCV;

    /// Tasks allocator variables:
    DynArray<Task> mTasks; // TODO growable fixed-size allocator
    //std::atomic<unsigned int> mTasksNum;
    Mutex mTaskListMutex;
    TaskID mFirstFreeTask;

    void SchedulerCallback(WorkerThread* thread);

    TaskID AllocateTask();
    void FreeTask(TaskID taskID);
    void FinishTask(TaskID taskID);
    void EnqueueTaskInternal(TaskID taskID);

    // create "num" additional worker threads
    void SpawnWorkerThreads(size_t num);

    bool ResizeTasksTable(uint32 newSize);

public:
    struct Setup
    {
        uint32 numInitialTasks;
        uint32 numThreads;

        Setup()
            : numInitialTasks(1 << 16)
            , numThreads(0)
        { }
    };

    ThreadPool(const Setup& setup = Setup());
    ~ThreadPool();

    /**
     * Get number of worker threads in the pool.
     */
    uint32 GetThreadsNumber() const;

    /**
     * Create a new task without enqueueing it.
     * @param desc     Task descriptor.
     * @remarks This function is thread-safe.
     */
    TaskID CreateTask(const TaskDesc& desc);

    /**
     * Enqueue a task (if dependency is resolved).
     * @remarks This function is thread-safe.
     */
    void EnqueueTask(TaskID taskID);

    /**
     * Check if a task is completed.
     */
    bool IsTaskFinished(TaskID taskID) const;

    /**
     * Waits for a task to finish. Can be called only for waitable tasks.
     */
    void WaitForTask(TaskID taskID);

    /**
     * Waits for multiple tasks to finish. Can be called only for waitable tasks.
     *
     * @param tasks    List of tasks to wait for.
     * @param tasksNum Number of tasks in @p tasks array.
     */
    void WaitForTasks(TaskID* tasks, size_t tasksNum);

    /**
     * Create a new task and enqueue it if dependency is resolved.
     * @param desc     Task descriptor.
     * @remarks This function is thread-safe.
     */
    TaskID CreateAndEnqueueTask(const TaskDesc& desc);
};

} // namespace Common
} // namespace NFE
