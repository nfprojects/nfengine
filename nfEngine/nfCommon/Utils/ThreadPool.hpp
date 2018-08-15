/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Thread pool classes declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/ConditionVariable.hpp"
#include "../Containers/UniquePtr.hpp"
#include "../Containers/DynArray.hpp"
#include "../Containers/Deque.hpp"

#include <inttypes.h>
#include <functional>
#include <atomic>
#include <thread>


namespace NFE {
namespace Common {

/**
 * Thread pool task unique identifier.
 */
using TaskID = uint32;

#define NFE_INVALID_TASK_ID (static_cast<NFE::Common::TaskID>(-1))

class ThreadPool;

struct TaskContext
{
    ThreadPool* pool;
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

public:
    enum class State : uint8
    {
        Created = 0,
        Scheduled,
        Enqueued,
        Finished,
    };

    Task();
    void Reset();

private:

    TaskFunction mCallback;  //< task routine

    /**
     * Number of tasks and sub-tasks left to complete.
     * If reaches 0, then whole task is finished.
     */
    std::atomic<uint32> mTasksLeft;
    TaskID mParent;

    /// Dependency pointers:
    TaskID mDependency;  //< dependency tasks ID
    TaskID mHead;        //< the first task that is dependent on this task
    TaskID mTail;        //< the last task that is dependent on this task
    TaskID mSibling;     //< the next task that is dependent on the same "mDependency" task
};

/**
 * @class WorkerThread
 * @brief Executor thread
 */
class WorkerThread
{
    friend class ThreadPool;

    std::thread mThread;
    bool mStarted; //< if set to false, exit the thread
    size_t mId;    //< thread number

    // force the class objects to occupy different cache lines
    char mPad[64];

public:
    WorkerThread(ThreadPool* pool, size_t id);
    ~WorkerThread();

private:
    WorkerThread(const WorkerThread&) = delete;
    WorkerThread& operator = (const WorkerThread&) = delete;
};

using WorkerThreadPtr = UniquePtr<WorkerThread>;


/**
 * @class ThreadPool
 * @brief Class enabling parallel tasks (user provided functions) execution.
 */
class NFCOMMON_API ThreadPool final
{
    friend class WorkerThread;

    /// Worker threads varibles:
    size_t mLastThreadId;
    DynArray<WorkerThreadPtr> mThreads;

    /// Tasks queue variables:
    size_t mMaxTasks;
    Deque<TaskID> mTasksQueue;        //< queue for tasks with "Queued" state
    Mutex mTasksQueueMutex;                 //< lock for "mTasksQueue" access
    ConditionVariable mTaskQueueCV;         //< CV for notifying about a new task in the queue

    Mutex mFinishedTasksMutex;
    ConditionVariable mFinishedTasksCV;

    /// Tasks allocator variables:
    std::atomic<uint32> mTasksNum;
    UniquePtr<Task[]> mTasks;

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
    ThreadPool(size_t maxTasks = (1 << 16) + 1, size_t threadsNum = 0);
    ~ThreadPool();

    /**
     * Get number of worker threads in the pool.
     */
    uint32 GetThreadsNumber() const;

    /**
     * Create a new task and enqueue it if dependency is resolved.
     *
     * @param function     Task routine.
     * @param parentID     Parent task ID.
     * @param dependencyID Dependency task ID.
     *
     * @remarks This function is thread-safe.
     */
    TaskID CreateTask(const TaskFunction& function,
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
