/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Thread pool classes declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/ConditionVariable.hpp"
#include "../Containers/UniquePtr.hpp"

#include <inttypes.h>
#include <set>
#include <queue>
#include <functional>
#include <atomic>
#include <thread>


namespace NFE {
namespace Common {

/**
 * Thread pool task unique identifier.
 */
using TaskID = uint32;

#define NFE_THREADPOOL_PRIORITY_BITS    2
#define NFE_THREADPOOL_PRIORITIES       (1 << NFE_THREADPOOL_PRIORITY_BITS)
#define NFE_INVALID_TASK_ID (static_cast<::NFE::Common::TaskID>(-1))

class ThreadPool;

struct TaskContext
{
    ThreadPool* pool;
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
 * @class Task
 * @brief Internal task structure.
 * @remarks Only @p ThreadPool class can access it.
 */
class Task final
{
public:

    enum class State : uint8
    {
        Invalid,
        Created,
        Queued,
        Finished,
    };

    friend class ThreadPool;

    TaskFunction mCallback;  //< task routine
    std::atomic<State> mState;

    /**
     * Number of tasks and sub-tasks left to complete.
     * If reaches 0, then whole task is finished.
     */
    std::atomic<uint32> mTasksLeft;

    /// Instances counters:
    uint32 mInstancesNum;                //< total number of the task instances
    uint32 mNextInstance;                //< next instance ID to execute
    std::atomic<uint32> mInstancesLeft;  //< number of instances left to complete

    union
    {
        TaskID mParent;
        TaskID mNextFree;   //< free tasks list
    };

    /// Dependency pointers:
    TaskID mDependency;  //< dependency tasks ID
    TaskID mHead;        //< the first task that is dependent on this task
    TaskID mTail;        //< the last task that is dependent on this task
    TaskID mSibling;     //< the next task that is dependent on the same "mDependency" task

    uint8 mPriority : NFE_THREADPOOL_PRIORITY_BITS;
    bool mWaitable  : 1;

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
    friend class ThreadPool;

    std::thread mThread;
    bool mStarted; //< if set to false, exit the thread
    size_t mId;    //< thread number

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

    /// Worker threads varibles:
    size_t mLastThreadId;
    std::set<WorkerThreadPtr> mThreads;

    /// Tasks queue variables:
    size_t mMaxTasks;

    // queues for tasks with "Queued" state
    std::queue<TaskID> mTasksQueues[NFE_THREADPOOL_PRIORITIES];
    Mutex mTasksQueueMutex;                 //< lock for "mTasksQueue" access
    ConditionVariable mTaskQueueCV;         //< CV for notifying about a new task in the queue

    Mutex mFinishedTasksMutex;
    ConditionVariable mFinishedTasksCV;

    /// Tasks allocator variables:
    UniquePtr<Task[]> mTasks;
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
    size_t GetThreadsNumber() const;

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
