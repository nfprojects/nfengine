/**
    NFEngine project

    \file   ThreadPool.h
    \brief  Thread pool classes declarations.
*/

#pragma once
#include "nfCommon.h"

namespace NFE {
namespace Common {

class ThreadPool;
typedef void (*TaskCallback)(void*, int, int); //user data, instance, thread ID
typedef long long TaskID;

/**
 * A threadpool task description.
 */
struct Task
{
    TaskID id;
    TaskCallback pCallback;
    void* pUserData;
    uint32 instancesCount;
    uint32 instancesLeft;

    Task(TaskID _id, TaskCallback _pCallback, void* _pUserData, uint32 _instancesCount) :
        id(_id), pCallback(_pCallback), pUserData(_pUserData), instancesCount(_instancesCount),
        instancesLeft(_instancesCount) {}

    /*
    // TODO: task priorities
    int priority;
    struct Compare
    {
        bool operator()(const Task& lhs, const Task& rhs) const
        {
            if (lhs.priority < lhs.priority)
                return true;
            else if (lhs.priority > lhs.priority)
                return false;
            else
                return (lhs.Id < rhs.Id);
        }
    };
    */
};

/**
 * Helper class containing information about executor thread.
 */
class ExecutorThread
{
    friend class ThreadPool;

private:
    ThreadPool* pThreadPool;    // owner
    volatile TaskID currTask;   // currenly executed task (if busy == true)
    std::thread thread;         // thread object
    volatile bool busy;         // is the thread executing a task?
    uint32 id;                  // thread id (counting from 0)

    ExecutorThread(const ExecutorThread&);

public:
    void* operator new(size_t size);
    void operator delete(void* ptr);

    ExecutorThread(ThreadPool* pTP, uint32 _id): busy(false), pThreadPool(pTP), id(_id) {};
};

/**
 * Thread pool manager.
 */
class NFCOMMON_API ThreadPool
{
private:
    typedef std::unique_lock<std::mutex> LockType;
    static void SchedulerThreadCallback(ExecutorThread* pThreadDesc);

    std::vector<ExecutorThread*> mThreads;

    // notifies the thread pool that a task finished a task
    std::condition_variable mTaskFinishedEvent;
    mutable std::mutex mTaskFinishedMutex;

    // mTaskQueue synchronization
    std::condition_variable mTaskQueueEvent;
    mutable std::mutex mTaskQueueMutex;
    std::queue<Task> mTaskQueue;

    std::atomic<bool> mStarted; // is thread pool running?
    std::atomic<TaskID> mLastTaskId;

    /// disable default methods
    ThreadPool(const ThreadPool&);
    ThreadPool& operator = (const ThreadPool&);

public:
    ThreadPool();
    ~ThreadPool();

    /**
     * Initialize thread pool.
     * @param threadsCount Threads count (use 0 to detect optimal value)
     */
    int Init(size_t threadsCount = 0);

    /**
     * Wait for currently executed tasks and release thread pool
     */
    void Release();

    /**
     * Get number of threads in the pool
     */
    const size_t GetThreadsCount() const;

    /**
     * Add a new task to the tasks queue. This function is thread-safe.
     * @param pCallback Task routine
     * @param pUserData Custom user data passed to the routine
     * @param instancesCount Number of task instances to spawn
     * @return 0 if failed to create task, else unique task identifier
     */
    TaskID AddTask(TaskCallback pCallback, void* pUserData = 0, uint32 instancesCount = 1);

    /**
     * Check if a task has been completed.
     */
    bool IsTaskDone(const TaskID task) const;

    /**
     * Check if all tasks has been completed.
     */
    bool AreAllTasksDone() const;

    /**
     * Wait until a task is completed. 
     * @remarks This function can't be called from task callback, because it could lead to a deadlock.
     */
    int WaitForTask(const TaskID task);

    /**
     * Wait until all tasks are completed.
     * @remarks This function can't be called from task callback, because it could lead to a deadlock.
     */
    int WaitForAllTasks();

    /**
     * Get thread pool load.
     * @return Active worker threads
     */
    size_t GetLoad() const;
};

} // namespace Common
} // namespace NFE
