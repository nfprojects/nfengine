#include "PCH.hpp"
#include "TaskBuilder.hpp"
#include "ThreadPool.hpp"


namespace NFE {
namespace Common {

TaskBuilder::TaskBuilder(const TaskID parentTask)
    : mParentTask(parentTask)
{
}

TaskBuilder::TaskBuilder(const TaskContext& taskContext)
    : mParentTask(taskContext.taskId)
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

void TaskBuilder::CustomTask(TaskID customTask)
{
    ThreadPool& tp = ThreadPool::GetInstance();

    TaskDesc desc;
    desc.debugName = "TaskBuilder::CustomTask";
    desc.parent = mParentTask;
    desc.dependency = customTask;

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

    struct NFE_ALIGN(64) ThreadData
    {
        uint32 elementOffset = 0; // base element
        uint32 numElements = 0;
        std::atomic<int32> counter = 0;
        uint32 threadDataIndex = 0;

        ThreadData() = default;
        ThreadData(const ThreadData& other)
            : elementOffset(other.elementOffset)
            , numElements(other.numElements)
            , counter(other.counter.load())
            , threadDataIndex(other.threadDataIndex)
        {}
    };

    // TODO get rid of dynamic allocation, e.g. by using some kind of pool
    using Counter = std::atomic<uint32>;
    using ThreadDataPtr = SharedPtr<DynArray<ThreadData>>;
    ThreadDataPtr threadDataPtr = MakeSharedPtr<DynArray<ThreadData>>();
    threadDataPtr->Resize(tp.GetNumThreads());

    // subdivide work
    {
        uint32 totalElements = 0;
        for (uint32 i = 0; i < numTasksToSpawn; ++i)
        {
            ThreadData& threadData = (*threadDataPtr)[i];
            threadData.numElements = (arraySize / numTasksToSpawn) + ((arraySize % numTasksToSpawn > i) ? 1 : 0);
            threadData.elementOffset = totalElements;
            totalElements += threadData.numElements;
        }
    }

    for (uint32 i = 0; i < numTasksToSpawn; ++i)
    {
        TaskDesc subTaskDesc;
        subTaskDesc.debugName = debugName;
        subTaskDesc.parent = parallelForTask;
        subTaskDesc.dependency = mDependencyTask;
        subTaskDesc.function = [func, threadDataPtr, numTasksToSpawn] (const TaskContext& context)
        {
            // consume elements assigned to each thread (starting from self)
            for (uint32 threadDataOffset = 0; threadDataOffset < numTasksToSpawn; ++threadDataOffset)
            {
                uint32 threadDataIndex = context.threadId + threadDataOffset;
                if (threadDataIndex >= numTasksToSpawn)
                {
                    threadDataIndex -= numTasksToSpawn;
                }

                ThreadData& threadData = threadDataPtr->Data()[threadDataIndex];

                uint32 index;
                while ((index = threadData.counter++) < threadData.numElements)
                {
                    func(context, static_cast<uint32>(threadData.elementOffset + index));
                }
            }
        };

        tp.CreateAndDispatchTask(subTaskDesc);
    }
}

} // namespace Common
} // namespace NFE
