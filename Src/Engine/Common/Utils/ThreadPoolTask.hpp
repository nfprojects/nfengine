/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Thread pool classes declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/Mutex.hpp"
#include "../System/ConditionVariable.hpp"
#include "../Containers/SharedPtr.hpp"

#include <functional>
#include <atomic>


namespace NFE {
namespace Common {

class Waitable;

/**
 * Thread pool task unique identifier.
 */
using TaskID = uint32;

static constexpr TaskID InvalidTaskID = UINT32_MAX;

// Function object representing a task.
using TaskFunction = std::function<void(const TaskContext& context)>;

// Parallel-for callback
using ParallelForTaskFunction = std::function<void(const TaskContext& context, uint32 arrayIndex)>;

/**
 * Task execution context.
 */
struct TaskContext
{
    ThreadPool* pool;
    uint32 threadId;    // thread ID (counted from 0)
    TaskID taskId;      // this task ID
};

/**
 * @brief Internal task structure.
 * @remarks Only @p ThreadPool class can access it.
 */
class Task final
{
    friend class ThreadPool;

public:

    enum class State : uint8
    {
        Invalid,    // unused task table entries are in invalid state
        Created,    // created task, waiting for a dependency to be fulfilled
        Queued,     // a task with fulfilled dependency, waiting in queue for execution
        Executing,  // task is being executed right now
        Finished,
    };

    static const uint8 Flag_IsDispatched = 1;
    static const uint8 Flag_DependencyFullfilled = 2;

    TaskFunction mCallback;  //< task routine

    std::atomic<State> mState;
    std::atomic<uint8> mDependencyState;

    // Number of sub-tasks left to complete.
    // If reaches 0, then whole task is considered as finished.
    std::atomic<int32> mTasksLeft;

    union
    {
        TaskID mParent;
        TaskID mNextFree;   //< free tasks list
    };

    // optional waitable object (it gets notified in the task is finished)
    Waitable* mWaitable;

    const char* mDebugName;

    // Dependency pointers:
    TaskID mDependency;  //< dependency tasks ID
    TaskID mHead;        //< the first task that is dependent on this task
    TaskID mTail;        //< the last task that is dependent on this task
    TaskID mSibling;     //< the next task that is dependent on the same "mDependency" task

    uint8 mPriority;

    // TODO: alignment

    Task();
    Task(Task&& other);
    void Reset();
};


} // namespace Common
} // namespace NFE
