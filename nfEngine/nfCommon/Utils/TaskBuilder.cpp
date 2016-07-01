#include "PCH.hpp"
#include "TaskBuilder.hpp"
#include "ThreadPool.hpp"
#include "../Containers/SharedPtr.hpp"


namespace NFE {
namespace Common {

TaskBuilder::TaskBuilder(const TaskID parentTask)
    : mParentTask(parentTask)
{
}

TaskBuilder::TaskBuilder(Waitable& waitable)
    : mWaitable(&waitable)
{
}

TaskBuilder::~TaskBuilder()
{
    ThreadPool& tp = ThreadPool::GetInstance();

    if (mWaitable)
    {
        Fence(mWaitable);
    }

    // flush previous dependency
    if (mDependencyTask != InvalidTaskID)
    {
        tp.DispatchTask(mDependencyTask);
        mDependencyTask = InvalidTaskID;
    }

    for (TaskID pendingTask : mPendingTasks)
    {
        tp.DispatchTask(pendingTask);
    }
}

void TaskBuilder::Fence(Waitable* waitable)
{
    ThreadPool& tp = ThreadPool::GetInstance();

    // flush previous dependency
    if (mDependencyTask != InvalidTaskID)
    {
        tp.DispatchTask(mDependencyTask);
        mDependencyTask = InvalidTaskID;
    }

    TaskDesc depDesc;
    depDesc.debugName = "TaskBuilder::Fence";
    depDesc.waitable = waitable;

    TaskID dependency = tp.CreateTask(depDesc);

    // flush pending tasks and link them to dependency task
    for (TaskID pendingTask : mPendingTasks)
    {
        TaskDesc desc;
        desc.debugName = "TaskBuilder::Fence/Sub";
        desc.parent = dependency;
        desc.dependency = pendingTask;
        tp.CreateAndDispatchTask(desc);

        tp.DispatchTask(pendingTask);
    }
    mPendingTasks.Clear();

    mDependencyTask = dependency;
}

void TaskBuilder::Task(const char* debugName, const TaskFunction& func)
{
    ThreadPool& tp = ThreadPool::GetInstance();

    TaskDesc desc;
    desc.function = func;
    desc.debugName = debugName;
    desc.parent = mParentTask;
    desc.dependency = mDependencyTask;

    TaskID taskID = tp.CreateTask(desc);
    mPendingTasks.PushBack(taskID);
}

void TaskBuilder::ParallelFor(const char* debugName, uint32 arraySize, const ParallelForTaskFunction& func)
{
    ThreadPool& tp = ThreadPool::GetInstance();

    if (arraySize == 0)
    {
        return;
    }

    TaskDesc desc;
    desc.debugName = debugName;
    desc.parent = mParentTask;
    desc.dependency = mDependencyTask;

    TaskID parallelForTask = tp.CreateTask(desc);
    mPendingTasks.PushBack(parallelForTask);

    uint32 numTasksToSpawn = Math::Min(arraySize, tp.GetNumThreads());

    // loop index counter
    // TODO get rid of dynamic allocation, e.g. by using some kind of pool
    using Counter = std::atomic<uint32>;
    using CounterPtr = SharedPtr<Counter>;
    CounterPtr counterPtr = MakeSharedPtr<Counter>(0);

    for (uint32 i = 0; i < numTasksToSpawn; ++i)
    {
        TaskDesc subTaskDesc;
        subTaskDesc.debugName = debugName;
        subTaskDesc.parent = parallelForTask;
        subTaskDesc.dependency = mDependencyTask;
        subTaskDesc.function = [func, counterPtr, arraySize] (const TaskContext& context)
        {
            Counter& counter = *counterPtr;

            uint32 index;
            while ((index = counter++) < arraySize)
            {
                func(context, index);
            }
        };

        tp.CreateAndDispatchTask(subTaskDesc);
    }
}

} // namespace Common
} // namespace NFE
