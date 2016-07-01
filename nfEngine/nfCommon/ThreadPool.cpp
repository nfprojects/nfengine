/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Thread pool classes declarations.
 */

#include "PCH.hpp"
#include "ThreadPool.hpp"
#include "Logger.hpp"

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
    mTasks.reset(new Task [mMaxTasks]);

    // initalize free list
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
        Lock lock(mTasksQueueMutex);

        for (auto thread : mThreads)
            thread->mStarted = false;

        mTaskQueueCV.notify_all();
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
    mTaskQueueCV.notify_all();
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
            Lock lock(mTasksQueueMutex);
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

                mTaskQueueCV.wait(lock);
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
        EnqueueTask(siblingID);
        siblingID = mTasks[siblingID].mSibling;
    }

    // update parent
    if (task.mParent != NFE_INVALID_TASK_ID)
        FinishTask(task.mParent);

    if (task.mWaitable)
    {
        task.mState = Task::State::Finished;

        // notify WaitForTask
        Lock lock(mFinishedTasksMutex);
        mFinishedTasksCV.notify_all();
    }
    else
        FreeTask(taskID);
}

void ThreadPool::EnqueueTask(TaskID taskID)
{
    Lock lock(mTasksQueueMutex);

    Task& task = mTasks[taskID];
    task.mState = Task::State::Queued;
    mTasksQueues[task.mPriority].push(taskID);
    mTaskQueueCV.notify_all();
}

void ThreadPool::FreeTask(TaskID taskID)
{
    assert(taskID < mMaxTasks);

    // TODO lockless list insert
    std::lock_guard<std::mutex> lock(mTaskListMutex);

    Task& task = mTasks[taskID];
    assert(task.mState != Task::State::Invalid);
    task.mState = Task::State::Invalid;
    task.mNextFree = mFirstFreeTask;
    mFirstFreeTask = taskID;
}

TaskID ThreadPool::AllocateTask()
{
    // TODO lockless list erase
    std::lock_guard<std::mutex> lock(mTaskListMutex);

    if (mFirstFreeTask == NFE_INVALID_TASK_ID)
        return NFE_INVALID_TASK_ID;

    Task& task = mTasks[mFirstFreeTask];
    assert(task.mState == Task::State::Invalid);
    task.mState = Task::State::Created;
    TaskID newNextFree = task.mNextFree;
    TaskID taskID = mFirstFreeTask;
    mFirstFreeTask = newNextFree;
    return taskID;
}

TaskID ThreadPool::CreateTask(const TaskFunction& function, size_t instancesNum,
                              TaskID parentID, TaskID dependencyID, bool waitable, unsigned char priority)
{
    assert(priority < NFE_THREADPOOL_PRIORITIES);

    if (instancesNum == 0)
        return NFE_INVALID_TASK_ID;

    TaskID taskID = AllocateTask();
    if (taskID == NFE_INVALID_TASK_ID)
        return NFE_INVALID_TASK_ID;

    Task& task = mTasks[taskID];
    task.Reset();
    task.mPriority = priority;
    task.mTasksLeft = 1;
    task.mCallback = function;
    task.mParent = parentID;
    task.mInstancesNum = task.mInstancesLeft = static_cast<uint32>(instancesNum);
    task.mDependency = dependencyID;
    task.mHead = NFE_INVALID_TASK_ID;
    task.mWaitable = waitable;
    task.mState = Task::State::Created;

    if (parentID != NFE_INVALID_TASK_ID)
        mTasks[parentID].mTasksLeft++;

    bool canEnqueue = true;
    if (dependencyID != NFE_INVALID_TASK_ID)
    {
        Task& dependency = mTasks[dependencyID];

        Lock lock(mFinishedTasksMutex); // TODO: how to get rid of it?
        if (dependency.mTasksLeft > 0)
        {
            // update dependency list
            if (dependency.mTail != NFE_INVALID_TASK_ID)
                mTasks[dependency.mTail].mSibling = taskID;
            else
                dependency.mHead = taskID;

            task.mDependency = dependencyID;
            dependency.mTail = taskID;

            canEnqueue = false;
        }
    }

    if (canEnqueue)
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
        assert(worker->mThread.get_id() != std::this_thread::get_id());
#endif

    Task& task = mTasks[taskID];

    assert(task.mWaitable);
    assert(task.mState != Task::State::Invalid);

    if (mTasks[taskID].mTasksLeft == 0)
        return;

    Lock lock(mFinishedTasksMutex);
    while (mTasks[taskID].mTasksLeft > 0)
        mFinishedTasksCV.wait(lock);

    // we can wait only once
    task.mWaitable = false;
    FreeTask(taskID);
}

void ThreadPool::WaitForTasks(TaskID* tasks, size_t tasksNum)
{
    for (size_t i = 0; i < tasksNum; ++i)
        WaitForTask(tasks[i]);
}

} // namespace Common
} // namespace NFE
