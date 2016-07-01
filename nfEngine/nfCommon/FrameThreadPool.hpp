/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Frame thread pool classes declarations.
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
 * Frame thread pool task unique identifier.
 */
typedef uint32 TaskID;

#define NFE_INVALID_TASK_ID (static_cast<NFE::Common::TaskID>(-1))

class FrameThreadPool;

struct TaskContext
{
    FrameThreadPool* pool;
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
 * @remarks Only @p FrameThreadPool class can access it.
 */
class Task final
{
    friend class FrameThreadPool;

    TaskFunction mCallback;  //< task routine

    /**
     * Number of tasks and sub-tasks left to complete.
     * If reaches 0, then whole task is finished.
     */
    std::atomic<uint32> mTasksLeft;
    TaskID mParent;

    /// Instances counters:
    uint32 mInstancesNum;                //< total number of the task instances
    uint32 mNextInstance;                //< next instance ID to execute
    std::atomic<uint32> mInstancesLeft;  //< number of instances left to complete

    /// Dependency pointers:
    TaskID mDependency;  //< dependency tasks ID
    TaskID mHead;        //< the first task that is dependent on this task
    TaskID mTail;        //< the last task that is dependent on this task
    TaskID mSibling;     //< the next task that is dependent on the same "mDependency" task

    // TODO: alignment

public:
    Task();
    void Reset();
};

/**
 * @class WorkerThread
 * @brief Executor thread
 */
class WorkerThread
{
    friend class FrameThreadPool;

    std::thread mThread;
    bool mStarted; //< if set to false, exit the thread
    size_t mId;    //< thread number

    // force the class objects to occupy different cache lines
    char mPad[64];

public:
    WorkerThread(FrameThreadPool* pool, size_t id);
    ~WorkerThread();
};

typedef std::shared_ptr<WorkerThread> WorkerThreadPtr;


/**
 * @class FrameThreadPool
 * @brief Class enabling parallel tasks (user provided functions) execution.
 */
class NFCOMMON_API FrameThreadPool final
{
    friend class WorkerThread;

    typedef std::unique_lock<std::mutex> Lock;

    /// Worker threads varibles:
    size_t mLastThreadId;
    std::set<WorkerThreadPtr> mThreads;

    /// Tasks queue variables:
    size_t mMaxTasks;
    std::queue<TaskID> mTasksQueue;        //< queue for tasks with "Queued" state
    std::mutex mTasksQueueMutex;           //< lock for "mTasksQueue" access
    std::condition_variable mTaskQueueCV;  //< CV for notifying about a new task in the queue

    std::mutex mFinishedTasksMutex;
    std::condition_variable mFinishedTasksCV;

    /// Tasks allocator variables:
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
    FrameThreadPool(size_t maxTasks = (1 << 16) + 1, size_t threadsNum = 0);
    ~FrameThreadPool();

    /**
     * Get number of worker threads in the pool.
     */
    size_t GetThreadsNumber() const;

    /**
     * Create a new task and enqueue it if dependency is resolved.
     *
     * @param function     Task routine.
     * @param instancesNum Number of the task instances.
     * @param parentID     Parent task ID.
     * @param dependencyID Dependency task ID.
     *
     * @remarks This function is thread-safe.
     */
    TaskID CreateTask(const TaskFunction& function,
                      size_t instancesNum = 1,
                      TaskID parentID = NFE_INVALID_TASK_ID,
                      TaskID dependencyID = NFE_INVALID_TASK_ID);

    /**
     * Check if a task is completed.
     */
    bool IsTaskFinished(TaskID taskID) const;

    /**
     * Waits for a task to finish.
     */
    void WaitForTask(TaskID taskID);

    /**
     * Waits for multiple tasks to finish.
     *
     * @param tasks    List of tasks to wait for.
     * @param tasksNum Number of tasks in @p tasks array.
     */
    void WaitForTasks(TaskID* tasks, size_t tasksNum);

    /**
     * Waits for all tasks in the pool to finish and reset tasks counter.
     */
    void WaitForAllTasks();
};

} // namespace Common
} // namespace NFE
