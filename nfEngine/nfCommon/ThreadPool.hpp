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
 * Function object representing a task.
 * @param instance Instance id of the whole task
 * @param thread   Thread id
 */
typedef std::function<void(size_t instance, size_t thread)> TaskFunction;

class ThreadPool;

enum class TaskState
{
    Invalid,   //< for not used tasks (already created and released with a wait method)
    Dependent, //< the task has unresolved dependency
    Queued,    //< the task is ready to execute
    Executed,  //< the task is being executed
    Finished   //< the task has finished and now needs to be released with a wait method
};

/**
 * @class Task
 * @brief Internal task structure.
 * @remarks Only @p ThreadPool class can access it.
 */
class Task final
{
    friend class ThreadPool;

    TaskState mState;

    /// task-related members
    TaskFunction mCallback;      //< task routine
    size_t mInstancesNum;        //< total number of the task instances
    size_t mNextInstance;        //< next instance ID to execute
    std::atomic<size_t> mInstancesLeft;

    std::set<Task*> mParents;    //< parent tasks
    std::set<Task*> mChildren;   //< child tasks
    size_t mRequired;            //< number of parent tasks left to dependency resolve

    void RemoveFromParents();

public:
    Task(TaskFunction callback, size_t instancesNum);
};

/**
 * Thread pool task unique identifier.
 */
typedef Task* TaskID;

/**
 * @class WorkerThread
 * @brief Executor thread
 */
class WorkerThread
{
    friend class ThreadPool;

    std::thread mThread;
    std::atomic<bool> mStarted; //< if set to false, exit the thread
    size_t mId;

    // force the class objects to occupy different cache lines
    char mPad[64];

public:
    WorkerThread(ThreadPool* pPool, size_t id);
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

    size_t mLastThreadId;
    std::set<WorkerThreadPtr> mThreads;
    std::mutex mThreadsMutex;               //< lock for "mThreads"

    std::mutex mDepsQueueMutex;             //< lock for task dependencies access (Task members)

    std::condition_variable mTaskQueueTask;
    std::queue<Task*>
    mTasksQueue;          //< Queue for task with resolved dependencies (with "Waiting" state)
    std::mutex mTasksQueueMutex;            //< lock for "mTasksQueue" access

    std::atomic<TaskID> mLastTaskId;
    std::map<TaskID, Task*> mTasks;
    std::mutex mTasksMutex;                 //< lock for "mTasks"
    std::condition_variable mTasksMutexCV;  //< condition variable used to notify about finished task

    // translate TaskID to Task object
    Task* GetTask(const TaskID& ptr) const;

    void SchedulerCallback(WorkerThread* thread);

    // create "num" additional worker threads
    void SpawnWorkerThreads(size_t num);

    // tell a worker thread to stop it's work
    void TriggerWorkerStop(WorkerThreadPtr workerThread);

    // worker thread routine
    void WorkerThreadCallback();

public:
    ThreadPool();
    ~ThreadPool();

    /**
     * Get number of worker threads in the pool.
     */
    size_t GetThreadsNumber() const;

    /**
     * Change number of worker threads.
     * @details When decreasing the actual value, task are not stopped - the threads that finish
                a task instances as first are destroyed.
     * @remarks This function is thread-safe.
     */
    void SetThreadsNumber(size_t newValue);

    /**
     * Create a new task.
     *
     * @remarks This function is thread-safe.
     * @param function        Task routine (can be null for creating synchronization point only).
     * @param instances       Number of task routine instances to run.
     * @param dependencies    List of dependent tasks.
     * @param dependenciesNum Number of dependent tasks.
     * @return Task ID
     */
    TaskID Enqueue(TaskFunction function, size_t instances = 1,
                   const TaskID* dependencies = nullptr, size_t dependenciesNum = 0,
                   bool noWait = false);

    bool AddDependency(TaskID taskId, const TaskID* dependencies, size_t dependenciesNum = 1);

    /**
     * Waits for an task to finish.
     */
    void WaitForTask(TaskID taskPtr);

    /**
     * Waits for multiple tasks to finish.
     *
     * @param tasks    List of tasks to wait for.
     * @param tasksNum Number of tasks.
     */
    void WaitForTasks(const TaskID* tasks = nullptr, size_t tasksNum = 0);

    /**
     * Waits for all tasks in the pool to finish.
     */
    void WaitForAllTasks();
};

} // namespace Common
} // namespace NFE
