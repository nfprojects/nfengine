/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Thread pool classes declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "ThreadPoolTask.hpp"
#include "../System/ConditionVariable.hpp"
#include "../System/Thread.hpp"
#include "../Containers/UniquePtr.hpp"
#include "../Containers/DynArray.hpp"
#include "../Containers/Deque.hpp"

#include <inttypes.h>


namespace NFE {
namespace Common {


// Structure describing task, used during Task creation.
struct TaskDesc
{
    TaskFunction function;

    // waitable object (optional)
    Waitable* waitable = nullptr;

    // parent task to append to (optional)
    TaskID parent = InvalidTaskID;

    // dependency task (optional)
    TaskID dependency = InvalidTaskID;

    // Specifies target queue
    // Task with higher priority are always popped from the queues first.
    // Valid range is 0...(ThreadPool::NumPriorities-1)
    uint8 priority = 1;

    const char* debugName = nullptr;

    // TODO limiting number of parallel running tasks of certain type
    // (eg. long-running resource loading tasks)

    NFE_INLINE TaskDesc() = default;
    NFE_INLINE TaskDesc(const TaskFunction& func) : function(func) { }
};

// Thread pool's worker thread
class WorkerThread
{
    friend class ThreadPool;

    NFE_MAKE_NONCOPYABLE(WorkerThread)
    NFE_MAKE_NONMOVEABLE(WorkerThread)

    Thread mThread;
    uint32 mId;                     // thread number
    std::atomic<bool> mStarted;     // if set to false, exit the thread

public:
    WorkerThread(ThreadPool* pool, uint32 id);
    ~WorkerThread();
};

using WorkerThreadPtr = UniquePtr<WorkerThread>;


/**
 * @class ThreadPool
 * @brief Class enabling parallel tasks execution.
 */
class NFCOMMON_API ThreadPool final
{
    friend class WorkerThread;

    NFE_MAKE_NONCOPYABLE(ThreadPool)

public:

    static constexpr uint32 TasksCapacity = 1024 * 128;
    static constexpr uint32 NumPriorities = 3;
    static constexpr uint32 MaxPriority = NumPriorities - 1;

    ThreadPool();
    ~ThreadPool();

    static ThreadPool& GetInstance();

    // Get number of worker threads in the pool.
    NFE_FORCE_INLINE uint32 GetNumThreads() const { return mThreads.Size(); }

    // Create a new task.
    // The task will not be queued immidiately - it has to be queued manually via DispatchTask call
    // NOTE This function is thread-safe.
    TaskID CreateTask(const TaskDesc& desc);

    // Dispatch a created task for being executed.
    // NOTE: Using the task ID after dispatching the task is undefined behaviour.
    void DispatchTask(const TaskID taskID);

    NFE_INLINE void CreateAndDispatchTask(const TaskDesc& desc)
    {
        DispatchTask(CreateTask(desc));
    }

private:

    void SchedulerCallback(WorkerThread* thread);

    TaskID AllocateTask_NoLock();
    void FreeTask_NoLock(TaskID taskID);
    void FinishTask(TaskID taskID);
    void EnqueueTaskInternal_NoLock(TaskID taskID);
    void OnTaskDependencyFullfilled_NoLock(TaskID taskID);

    // create "num" additional worker threads
    void SpawnWorkerThreads(uint32 num);

    bool InitTasksTable(uint32 newSize);

    // Worker threads variables:
    DynArray<WorkerThreadPtr> mThreads;

    // queues for tasks with "Queued" state
    Deque<TaskID> mTasksQueues[NumPriorities];
    Mutex mTasksQueueMutex;                 //< lock for "mTasksQueue" access
    ConditionVariable mTaskQueueCV;         //< CV for notifying about a new task in the queue

    Mutex mTaskListMutex;
    DynArray<Task> mTasks; // TODO growable fixed-size allocator
    TaskID mFirstFreeTask;
};

} // namespace Common
} // namespace NFE
