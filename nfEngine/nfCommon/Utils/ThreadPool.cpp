/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Thread pool classes declarations.
 */

#include "PCH.hpp"
#include "ThreadPool.hpp"
#include "ScopedLock.hpp"
#include "Logger/Logger.hpp"

namespace NFE {
namespace Common {

Task::Task()
{
    Reset();
}

void Task::Reset()
{
    mState = State::Invalid;
    mTasksLeft = 0;
    mParent = NFE_INVALID_TASK_ID;
    mInstancesNum = 0;
    mNextInstance = 0;
    mInstancesLeft = 0;
    mDependency = NFE_INVALID_TASK_ID;
    mHead = NFE_INVALID_TASK_ID;
    mTail = NFE_INVALID_TASK_ID;
    mSibling = NFE_INVALID_TASK_ID;
    mWaitable = false;
}


WorkerThread::WorkerThread(ThreadPool* pool, size_t id)
{
    mStarted = true;
    this->mId = id;
    mThread = std::thread(&ThreadPool::SchedulerCallback, pool, this);
}

WorkerThread::~WorkerThread()
{
    mThread.join();
}


ThreadPool::ThreadPool(size_t maxTasks, size_t threadsNum)
    : mLastThreadId(0)
    , mMaxTasks(maxTasks)
{
    // allocate tasks buffer
    mTasks.Reset(new Task [mMaxTasks]);

    // initialize free list
    for (size_t i = 0; i < mMaxTasks - 1; ++i)
        mTasks[i].mNextFree = static_cast<TaskID>(i + 1);
    mTasks[mMaxTasks - 1].mNextFree = NFE_INVALID_TASK_ID;
    mFirstFreeTask = 0;

    if (threadsNum > 0)
        SpawnWorkerThreads(threadsNum);
    else
        SpawnWorkerThreads(std::thread::hardware_concurrency());
}

ThreadPool::~ThreadPool()
{
    {
        ScopedMutexLock lock(mTasksQueueMutex);

        for (auto thread : mThreads)
            thread->mStarted = false;

        mTaskQueueCV.SignalAll();
    }

    // cleanup
    mThreads.clear();
}

void ThreadPool::SpawnWorkerThreads(size_t num)
{
    for (size_t i = 0; i < num; ++i)
    {
        mThreads.insert(WorkerThreadPtr(new WorkerThread(this, mLastThreadId++)));
    }
}

void ThreadPool::TriggerWorkerStop(WorkerThreadPtr workerThread)
{
    workerThread->mStarted = false;
    mTaskQueueCV.SignalAll();
}

size_t ThreadPool::GetThreadsNumber() const
{
    return mThreads.size();
}

void ThreadPool::SchedulerCallback(WorkerThread* thread)
{
    Task* task = nullptr;
    TaskContext context;
    context.threadId = thread->mId;
    context.pool = this;

    for (;;)
    {
        {
            ScopedMutexLock lock(mTasksQueueMutex);
            std::queue<TaskID>* queue = nullptr;

            // wait for new task
            while (thread->mStarted)
            {
                // find queue with pending tasks
                queue = nullptr;
                for (size_t i = 0; i < NFE_THREADPOOL_PRIORITIES; ++i)
                    if (!mTasksQueues[i].empty())
                    {
                        queue = &mTasksQueues[i];
                        break;
                    }

                if (queue != nullptr)
                    break;

                mTaskQueueCV.Wait(lock);
            }

            if (!thread->mStarted)
                return;

            // pop a task from the queue
            context.taskId = queue->front();
            task = &mTasks[context.taskId];
            context.instanceId = task->mNextInstance++;
            if (task->mNextInstance == task->mInstancesNum)
                queue->pop();
        }

        // execute
        task->mCallback(context);

        // check if task has been completed (last instance just finished its execution)
        if (--task->mInstancesLeft == 0)
            FinishTask(context.taskId);
    }
}

void ThreadPool::FinishTask(TaskID taskID)
{
    Task& task = mTasks[taskID];

    if (--task.mTasksLeft > 0)
        return;

    // enqueue dependent tasks
    TaskID siblingID = task.mHead;
    while (siblingID != NFE_INVALID_TASK_ID)
    {
        EnqueueTaskInternal(siblingID);
        siblingID = mTasks[siblingID].mSibling;
    }

    // update parent
    if (task.mParent != NFE_INVALID_TASK_ID)
        FinishTask(task.mParent);

    if (task.mWaitable)
    {
        task.mState = Task::State::Finished;

        // notify WaitForTask
        ScopedMutexLock lock(mFinishedTasksMutex);
        mFinishedTasksCV.SignalAll();
    }
    else
        FreeTask(taskID);
}

void ThreadPool::EnqueueTaskInternal(TaskID taskID)
{
    ScopedMutexLock lock(mTasksQueueMutex);

    Task& task = mTasks[taskID];
    task.mState = Task::State::Queued;
    mTasksQueues[task.mPriority].push(taskID);
    mTaskQueueCV.SignalAll();
}

void ThreadPool::FreeTask(TaskID taskID)
{
    NFE_ASSERT(taskID < mMaxTasks, "Invalid task ID");

    // TODO lockless list insert
    ScopedMutexLock lock(mTaskListMutex);

    Task& task = mTasks[taskID];
    NFE_ASSERT(task.mState != Task::State::Invalid, "Invalid task state");
    task.mState = Task::State::Invalid;
    task.mNextFree = mFirstFreeTask;
    mFirstFreeTask = taskID;
}

TaskID ThreadPool::AllocateTask()
{
    // TODO lockless list erase
    ScopedMutexLock lock(mTaskListMutex);

    if (mFirstFreeTask == NFE_INVALID_TASK_ID)
        return NFE_INVALID_TASK_ID;

    Task& task = mTasks[mFirstFreeTask];
    NFE_ASSERT(task.mState == Task::State::Invalid, "Invalid task state");
    task.mState = Task::State::Created;
    TaskID newNextFree = task.mNextFree;
    TaskID taskID = mFirstFreeTask;
    mFirstFreeTask = newNextFree;
    return taskID;
}

TaskID ThreadPool::CreateTask(const TaskDesc& desc)
{
    NFE_ASSERT(desc.priority < NFE_THREADPOOL_PRIORITIES, "Invalid priority");

    if (desc.instancesNum == 0)
        return NFE_INVALID_TASK_ID;

    TaskID taskID = AllocateTask();
    if (taskID == NFE_INVALID_TASK_ID)
        return NFE_INVALID_TASK_ID;

    Task& task = mTasks[taskID];
    task.Reset();
    task.mPriority = desc.priority;
    task.mTasksLeft = 1;
    task.mCallback = desc.function;
    task.mParent = desc.parent;
    task.mInstancesNum = task.mInstancesLeft = desc.instancesNum;
    task.mDependency = desc.dependency;
    task.mHead = NFE_INVALID_TASK_ID;
    task.mWaitable = desc.waitable;
    task.mState = Task::State::Created;

    if (desc.parent != NFE_INVALID_TASK_ID)
        mTasks[desc.parent].mTasksLeft++;

    return taskID;
}

void ThreadPool::EnqueueTask(TaskID taskID)
{
    TaskID dependencyID = mTasks[taskID].mDependency;

    bool canEnqueue = true;
    if (dependencyID != NFE_INVALID_TASK_ID)
    {
        Task& dependency = mTasks[dependencyID];

        ScopedMutexLock lock(mFinishedTasksMutex); // TODO: how to get rid of it?
        if (dependency.mTasksLeft > 0)
        {
            // update dependency list
            if (dependency.mTail != NFE_INVALID_TASK_ID)
                mTasks[dependency.mTail].mSibling = taskID;
            else
                dependency.mHead = taskID;

            dependency.mTail = taskID;
            canEnqueue = false;
        }
    }

    if (canEnqueue)
    {
        EnqueueTaskInternal(taskID);
    }
}

TaskID ThreadPool::CreateAndEnqueueTask(const TaskDesc& desc)
{
    TaskID taskID = CreateTask(desc);
    if (taskID != NFE_INVALID_TASK_ID)
        EnqueueTask(taskID);
    return taskID;
}

bool ThreadPool::IsTaskFinished(TaskID taskID) const
{
    return mTasks[taskID].mTasksLeft == 0;
}

void ThreadPool::WaitForTask(TaskID taskID)
{
    // TODO use Thread helper class
    // waiting inside task is forbidden
#ifdef _DEBUG
    for (const auto& worker : mThreads)
        NFE_ASSERT(worker->mThread.get_id() != std::this_thread::get_id(),
                   "Waiting for a task inside worker thread may end with deadlock");
#endif

    Task& task = mTasks[taskID];

    NFE_ASSERT(task.mWaitable, "WaitForTask must be called on waitable task");
    NFE_ASSERT(task.mState != Task::State::Invalid, "Task can be waited for only once");

    if (mTasks[taskID].mTasksLeft == 0)
        return;

    ScopedMutexLock lock(mFinishedTasksMutex);
    while (mTasks[taskID].mTasksLeft > 0)
    {
        mFinishedTasksCV.Wait(lock);
    }

    // we can wait only once
    task.mWaitable = false;
    FreeTask(taskID);
}

void ThreadPool::WaitForTasks(TaskID* tasks, size_t tasksNum)
{
    for (size_t i = 0; i < tasksNum; ++i)
    {
        WaitForTask(tasks[i]);
    }
}

} // namespace Common
} // namespace NFE
