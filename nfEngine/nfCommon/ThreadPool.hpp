/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Thread pool classes declarations.
 */

#pragma once

#include "nfCommon.hpp"

#include <condition_variable>
#include <set>
#include <functional>
#include <inttypes.h>

namespace NFE {
namespace Common {

/**
 * Thread pool task unique identifier.
 */
typedef uint32 TaskID;

#define NFE_INVALID_TASK_ID (static_cast<TaskID>(-1))

struct TaskContext
{
    size_t instanceId;
    size_t threadId;
    TaskID taskId;
};

/**
 * Function object representing a task.
 * @param thread   Thread id
 */
typedef std::function<void(const TaskContext& context)> TaskFunction;

/**
 * @class Task
 * @brief Internal task structure.
 * @remarks Only @p ThreadPool class can access it.
 */
class Task final
{
    friend class ThreadPool;

    enum class State : uint32
    {
        Created,
        Dependent,
        Queued,
        Finished
    };

    TaskFunction mCallback;  //< task routine

    /**
     * Number of tasks and sub-tasks left to complete.
     * If reaches 0, then whole task is turned into "Finished".
     */
    uint32 mTasksLeft;
    State mState;
    TaskID mParent;

    /// Instances counters:
    uint32 mInstancesNum;                //< total number of the task instances
    uint32 mNextInstance;                //< next instance ID to execute
    std::atomic<uint32> mInstancesLeft;

    /// Dependency pointers:
    TaskID mDependency;  //< dependency tasks ID
    TaskID mHead;        //< the first task that is dependent on this task
    TaskID mTail;        //< the last task that is dependent on this task
    TaskID mSibling;     //< the next task that is dependent on the same "mDependency" task

    // TODO: alignment

public:
    Task();
};

class ThreadPool;

/**
 * @class WorkerThread
 * @brief Executor thread
 */
class WorkerThread
{
    friend class ThreadPool;

    std::thread mThread;
    bool mStarted; //< if set to false, exit the thread
    size_t mId;  //< thread number

    // force the class objects to occupy different cache lines
    char mPad[64];

public:
    WorkerThread(ThreadPool* pool, size_t id);
    ~WorkerThread();
};

typedef std::shared_ptr<WorkerThread> WorkerThreadPtr;


/**
 * @class ThreadPool
 * @brief Class enabling parallel tasks (user provided functions) execution.
 */
class NFCOMMON_API ThreadPool final
{
    friend class WorkerThread;

    typedef std::unique_lock<std::mutex> Lock;

    /// Worker threads varibles:
    size_t mLastThreadId;
    std::set<WorkerThreadPtr> mThreads;

    /// Tasks queue variables:
    std::queue<TaskID> mTasksQueue;        //< queue for tasks with "Queued" state
    std::mutex mTasksQueueMutex;           //< lock for "mTasksQueue" access
    std::condition_variable mTaskQueueCV;  //< CV for notifying about a new task in the queue

    std::mutex mFinishedTasksMutex;
    std::condition_variable mFinishedTasksCV;

    /// Tasks allocator variables:
    const uint32 MAX_TASKS_NUM = (1 << 16) + 10;
    std::atomic<uint32> mTasksNum;
    std::unique_ptr<Task[]> mTasks;

    void SchedulerCallback(WorkerThread* thread);

    void FinishTask(Task* task);
    void EnqueueTask(TaskID taskID);

    // create "num" additional worker threads
    void SpawnWorkerThreads(size_t num);

    // tell a worker thread to stop it's work
    void TriggerWorkerStop(WorkerThreadPtr workerThread);

    // worker thread routine
    void WorkerThreadCallback();

public:
    ThreadPool(size_t threadsNum = 0);
    ~ThreadPool();

    /**
     * Get number of worker threads in the pool.
     */
    size_t GetThreadsNumber() const;

    /**
     * Create a new task.
     *
     * @remarks This function is thread-safe.
     * @param function     Task routine (can be null for creating synchronization point only).
     */
    TaskID CreateTask(TaskFunction function,
                      size_t instancesNum = 1,
                      TaskID parentID = NFE_INVALID_TASK_ID,
                      TaskID dependencyID = NFE_INVALID_TASK_ID);

    /**
     * Check if a task is completed.
     */
    bool IsTaskFinished(TaskID taskID) const;

    /**
     * Waits for an task to finish.
     */
    void WaitForTask(TaskID taskID);

    /**
     * Waits for multiple tasks to finish.
     *
     * @param tasks    List of tasks to wait for.
     */
    void WaitForTasks(TaskID* tasks, size_t tasksNum);

    /**
     * Waits for all tasks in the pool to finish.
     */
    void WaitForAllTasks();
};

} // namespace Common
} // namespace NFE
