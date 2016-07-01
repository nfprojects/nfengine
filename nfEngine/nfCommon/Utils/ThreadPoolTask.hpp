/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Thread pool classes declarations.
 */

#pragma once

#include "../nfCommon.hpp"

#include <functional>
#include <atomic>


namespace NFE {
namespace Common {

/**
 * Thread pool task unique identifier.
 */
using TaskID = uint32;

/**
 * Function object representing a task.
 * @param thread   Thread id
 */
using TaskFunction = std::function<void(const TaskContext& context)>;

/**
 * Task execution context.
 */
struct TaskContext
{
    ThreadPool* pool;
    uint32 instanceId;  // task instance index
    uint32 threadId;    // thread ID (counted from 0)
    TaskID taskId;      // this task ID
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

    uint8 mPriority;
    bool mWaitable;

    // TODO: alignment

    Task();
    Task(Task&& other);
    void Reset();
};


/**
 * Helper class allowing for waiting for thread pool tasks.
 */
class TaskFence
{
public:

private:
    bool mFinished;
};


} // namespace Common
} // namespace NFE
