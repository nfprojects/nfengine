#pragma once

#include "../nfCommon.hpp"
#include "ThreadPoolTask.hpp"
#include "../Containers/StaticArray.hpp"

namespace NFE {
namespace Common {

// helper class that allows easy task-graph building
class NFCOMMON_API TaskBuilder
{
    NFE_MAKE_NONCOPYABLE(TaskBuilder)
    NFE_MAKE_NONMOVEABLE(TaskBuilder)

public:
    static constexpr uint32 MaxTasks = 128;

    TaskBuilder(const TaskID parentTask = InvalidTaskID);
    explicit TaskBuilder(const TaskContext& taskContext);
    explicit TaskBuilder(Waitable& waitable);
    ~TaskBuilder();

    // push a new task
    // Note: multiple pushed tasks can run in parallel
    void Task(const char* debugName, const TaskFunction& func);

    // Push a custom task
    // Note: The task must be created, but not yet dispatched
    void CustomTask(TaskID customTask);

    // push parallel-for task
    void ParallelFor(const char* debugName, uint32 arraySize, const ParallelForTaskFunction& func);

    // Push a sync point
    // All tasks pushed after the fence will start only when all the tasks pushed before the fence finish execution
    // Optionally signals waitable object
    void Fence(Waitable* waitable = nullptr);

private:
    // forbid dynamic allocation (only stack allocation is allowed)
    void* operator new(size_t size) = delete;
    void operator delete(void* ptr) = delete;
    void* operator new[] (size_t size) = delete;
    void operator delete[] (void* ptr) = delete;
    void* operator new(size_t size, void* ptr) = delete;
    void* operator new[] (size_t size, void* ptr) = delete;

    Waitable* mWaitable = nullptr;
    const TaskID mParentTask = InvalidTaskID;
    TaskID mDependencyTask = InvalidTaskID;

    // tasks that has to be synchronized after instering a fence or synchronizing with waitable object
    StaticArray<TaskID, MaxTasks> mPendingTasks;
};

} // namespace Common
} // namespace NFE
