/**
 * @file
 * @author Witek902
 * @brief  Thread pool classes declarations.
 */

#include "PCH.hpp"
#include "ThreadPool.hpp"
#include "Waitable.hpp"

namespace NFE {
namespace Common {

WorkerThread::WorkerThread(ThreadPool* pool, uint32 id)
    : mId(id)
    , mStarted(true)
{
    mThread.Run(&ThreadPool::SchedulerCallback, pool, this);

    char threadName[64];
    snprintf(threadName, sizeof(threadName), "NFE::Common::ThreadPool worker #%u", id);
    mThread.SetName(threadName);
}

WorkerThread::~WorkerThread()
{
}

//////////////////////////////////////////////////////////////////////////

static ThreadPool gThreadPool;

ThreadPool& ThreadPool::GetInstance()
{
    return gThreadPool;
}

ThreadPool::ThreadPool()
    : mFirstFreeTask(InvalidTaskID)
{
    // TODO make it configurable
    InitTasksTable(TasksCapacity);
    SpawnWorkerThreads(Thread::GetSystemThreadsCount());
}

ThreadPool::~ThreadPool()
{
    {
        NFE_SCOPED_LOCK(mTasksQueueMutex);

        for (const WorkerThreadPtr& thread : mThreads)
        {
            thread->mStarted = false;
        }

        mTaskQueueCV.SignalAll();
    }

    // cleanup
    mThreads.Clear();
}

bool ThreadPool::InitTasksTable(uint32 newSize)
{
    /*
    const uint32 oldSize = mTasks.Size();
    newSize = Math::Max(1u, newSize);

    if (oldSize < newSize)
    {
        if (!mTasks.Reserve(newSize))
        {
            NFE_LOG_ERROR("Failed to reserve space for threadpool's tasks buffer");
            return false;
        }

        uint32 lastFreeTask = mFirstFreeTask;
        for (uint32 i = oldSize; i < newSize; ++i)
        {
            Task newTask;
            newTask.mNextFree = lastFreeTask;
            lastFreeTask = i;

            mTasks.PushBack(std::move(newTask));
        }
        mFirstFreeTask = lastFreeTask;
    }
    */

    if (!mTasks.Resize(newSize))
    {
        NFE_LOG_ERROR("Failed to reserve space for threadpool's tasks buffer");
        return false;
    }

    mFirstFreeTask = 0;
    for (uint32 i = 0; i < newSize - 1; ++i)
    {
        mTasks[i].mNextFree = i + 1;
    }
    mTasks[newSize - 1].mNextFree = InvalidTaskID;

    return true;
}

void ThreadPool::SpawnWorkerThreads(uint32 num)
{
    for (uint32 i = 0; i < num; ++i)
    {
        mThreads.PushBack(MakeUniquePtr<WorkerThread>(this, i));
    }
}

void ThreadPool::SchedulerCallback(WorkerThread* thread)
{
    TaskContext context;
    context.pool = this;
    context.threadId = thread->mId;

    for (;;)
    {
        Task* task = nullptr;
        {
            ScopedExclusiveLock<Mutex> lock(mTasksQueueMutex);

            Deque<TaskID>* queue = nullptr;

            // wait for new task
            while (thread->mStarted)
            {
                // find queue with pending tasks
                queue = nullptr;
                for (uint32 i = 0; i < NumPriorities; ++i)
                {
                    if (!mTasksQueues[i].Empty())
                    {
                        queue = &mTasksQueues[i];
                        break;
                    }
                }

                if (queue != nullptr)
                {
                    break;
                }

                mTaskQueueCV.Wait(lock);
            }

            if (!thread->mStarted)
            {
                break;
            }

            // pop a task from the queue
            context.taskId = queue->Front();
            task = &mTasks[context.taskId];
            queue->PopFront();
        }

        if (task->mCallback)
        {
            // Queued -> Executing
            {
                const Task::State oldState = task->mState.exchange(Task::State::Executing);
                NFE_ASSERT(Task::State::Queued == oldState, "Task is expected to be in 'Queued' state");
            }

            // execute
            task->mCallback(context);

            // Executing -> Finished
            {
                const Task::State oldState = task->mState.exchange(Task::State::Finished);
                NFE_ASSERT(Task::State::Executing == oldState, "Task is expected to be in 'Executing' state");
            }
        }
        else
        {
            // Queued -> Finished

            const Task::State oldState = task->mState.exchange(Task::State::Finished);
            NFE_ASSERT(Task::State::Queued == oldState, "Task is expected to be in 'Queued' state");
        }

        FinishTask(context.taskId);
    }
}

void ThreadPool::FinishTask(TaskID taskID)
{
    TaskID taskToFinish = taskID;

    // Note: loop instead of recursion to avoid stack overflow in case of long dependency chains
    while (taskToFinish != InvalidTaskID)
    {
        TaskID parentTask;
        Waitable* waitable = nullptr;

        {
            NFE_SCOPED_LOCK(mTaskListMutex); // TODO lockless

            Task& task = mTasks[taskToFinish];

            parentTask = task.mParent;
            waitable = task.mWaitable;

            const int32 tasksLeft = --task.mTasksLeft;
            NFE_ASSERT(tasksLeft >= 0, "Tasks counter underflow");
            if (tasksLeft > 0)
            {
                return;
            }

            // notify about fullfilling the dependency
            {
                TaskID siblingID = task.mHead;
                while (siblingID != InvalidTaskID)
                {
                    OnTaskDependencyFullfilled_NoLock(siblingID);
                    siblingID = mTasks[siblingID].mSibling;
                }
            }

            FreeTask_NoLock(taskToFinish);
        }

        // notify waitable object
        if (waitable)
        {
            waitable->OnFinished();
        }

        // update parent (without recursion)
        taskToFinish = parentTask;
    }
}

void ThreadPool::EnqueueTaskInternal_NoLock(TaskID taskID)
{
    Task& task = mTasks[taskID];

    const Task::State oldState = task.mState.exchange(Task::State::Queued);
    NFE_ASSERT(Task::State::Created == oldState, "Task is expected to be in 'Created' state");
    NFE_ASSERT((Task::Flag_IsDispatched | Task::Flag_DependencyFullfilled) == task.mDependencyState, "Invalid dependency state");

    // push to queue
    {
        ScopedExclusiveLock<Mutex> lock(mTasksQueueMutex);
        mTasksQueues[task.mPriority].PushBack(taskID);
        mTaskQueueCV.SignalAll();
    }
}

void ThreadPool::FreeTask_NoLock(TaskID taskID)
{
    NFE_ASSERT(taskID < mTasks.Size(), "Invalid task ID");

    Task& task = mTasks[taskID];

    const Task::State oldState = task.mState.exchange(Task::State::Invalid);
    NFE_ASSERT(Task::State::Finished == oldState, "Task is expected to be in 'Finished' state");

    task.mNextFree = mFirstFreeTask;
    mFirstFreeTask = taskID;
}

TaskID ThreadPool::AllocateTask_NoLock()
{
    if (mFirstFreeTask == InvalidTaskID)
    {
        return InvalidTaskID;
    }

    Task& task = mTasks[mFirstFreeTask];

    const Task::State oldState = task.mState.exchange(Task::State::Queued);
    NFE_ASSERT(Task::State::Invalid == oldState, "Task is expected to be in 'Invalid' state");

    TaskID newNextFree = task.mNextFree;
    TaskID taskID = mFirstFreeTask;
    mFirstFreeTask = newNextFree;
    return taskID;
}

TaskID ThreadPool::CreateTask(const TaskDesc& desc)
{
    NFE_ASSERT(desc.priority < NumPriorities, "Invalid priority");

    // TODO lockless
    NFE_SCOPED_LOCK(mTaskListMutex);

    TaskID taskID = AllocateTask_NoLock();
    NFE_ASSERT(taskID != InvalidTaskID, "Failed to allocate task - is threadpool full?");

    if (taskID == InvalidTaskID)
    {
        return InvalidTaskID;
    }

    Task& task = mTasks[taskID];
    task.Reset();
    task.mDependencyState = 0;
    task.mPriority = desc.priority;
    task.mTasksLeft = 1;
    task.mCallback = desc.function;
    task.mParent = desc.parent;
    task.mDependency = desc.dependency;
    task.mWaitable = desc.waitable;
    task.mHead = InvalidTaskID;
    task.mState = Task::State::Created;
    task.mDebugName = desc.debugName;

    if (desc.parent != InvalidTaskID)
    {
        mTasks[desc.parent].mTasksLeft++;
    }

    TaskID dependencyID = mTasks[taskID].mDependency;

    bool dependencyFullfilled = true;
    if (dependencyID != InvalidTaskID)
    {
        Task& dependency = mTasks[dependencyID];

        NFE_ASSERT(Task::State::Invalid != dependency.mState, "Invalid state of dependency task");

        //NFE_SCOPED_LOCK(mFinishedTasksMutex); // TODO: how to get rid of it?
        if (dependency.mTasksLeft > 0)
        {
            // update dependency list
            if (dependency.mTail != InvalidTaskID)
            {
                mTasks[dependency.mTail].mSibling = taskID;
            }
            else
            {
                dependency.mHead = taskID;
            }

            dependency.mTail = taskID;
            dependencyFullfilled = false;
        }
    }

    if (dependencyFullfilled)
    {
        task.mDependencyState = Task::Flag_DependencyFullfilled;
    }

    return taskID;
}

void ThreadPool::DispatchTask(TaskID taskID)
{
    NFE_ASSERT(taskID != InvalidTaskID, "Invalid task");

    // TODO lockless
    NFE_SCOPED_LOCK(mTaskListMutex);

    Task& task = mTasks[taskID];

    NFE_ASSERT(Task::State::Created == task.mState, "Task is expected to be in 'Created' state");

    const uint8 oldDependencyState = task.mDependencyState.fetch_or(Task::Flag_IsDispatched);

    NFE_ASSERT((oldDependencyState & Task::Flag_IsDispatched) == 0, "Task already dispatched");

    // can enqueue only if not dispatched yet, but dependency was fullfilled
    if (Task::Flag_DependencyFullfilled == oldDependencyState)
    {
        EnqueueTaskInternal_NoLock(taskID);
    }
}

void ThreadPool::OnTaskDependencyFullfilled_NoLock(TaskID taskID)
{
    NFE_ASSERT(taskID != InvalidTaskID, "Invalid task");

    Task& task = mTasks[taskID];

    NFE_ASSERT(Task::State::Created == task.mState, "Task is expected to be in 'Created' state");

    const uint8 oldDependencyState = task.mDependencyState.fetch_or(Task::Flag_DependencyFullfilled);

    NFE_ASSERT((oldDependencyState & Task::Flag_DependencyFullfilled) == 0, "Task should not have dependency fullfilled");

    // can enqueue only if was dispatched
    if (Task::Flag_IsDispatched == oldDependencyState)
    {
        EnqueueTaskInternal_NoLock(taskID);
    }
}


} // namespace Common
} // namespace NFE
