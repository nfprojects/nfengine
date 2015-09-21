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
    mTasksLeft = 0;
    mParent = NFE_INVALID_TASK_ID;
    mInstancesNum = 0;
    mNextInstance = 0;
    mInstancesLeft = 0;
    mDependency = NFE_INVALID_TASK_ID;
    mHead = NFE_INVALID_TASK_ID;
    mTail = NFE_INVALID_TASK_ID;
    mSibling = NFE_INVALID_TASK_ID;
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
    , mMaxTasks(static_cast<uint32>(maxTasks))
    , mTasksNum(0)
{
    // allocate tasks buffer
    mTasks.reset(new Task [mMaxTasks]);

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

            // wait for new task
            while (thread->mStarted && mTasksQueue.empty())
                mTaskQueueCV.wait(lock);

            if (!thread->mStarted)
                return;

            // pop a task from the queue
            context.taskId = mTasksQueue.front();
            task = &mTasks[context.taskId];
            context.instanceId = task->mNextInstance++;
            if (task->mNextInstance == task->mInstancesNum)
                mTasksQueue.pop();
        }

        // execute
        task->mCallback(context);

        // check if task has been completed (last instance just finished its execution)
        if (--task->mInstancesLeft == 0)
            FinishTask(task);
    }
}

void ThreadPool::FinishTask(Task* task)
{
    if (--task->mTasksLeft > 0)
        return;

    {
        // TODO: get rid of this lock
        Lock lock(mFinishedTasksMutex);
        mFinishedTasksCV.notify_all();
    }

    // update parent
    if (task->mParent != NFE_INVALID_TASK_ID)
        FinishTask(&mTasks[task->mParent]);

    // enqueue dependent tasks
    TaskID siblingID = task->mHead;
    while (siblingID != NFE_INVALID_TASK_ID)
    {
        EnqueueTask(siblingID);
        siblingID = mTasks[siblingID].mSibling;
    }
}

void ThreadPool::EnqueueTask(TaskID taskID)
{
    Lock lock(mTasksQueueMutex);
    mTasksQueue.push(taskID);
    mTaskQueueCV.notify_all();
}

TaskID ThreadPool::CreateTask(const TaskFunction& function, size_t instancesNum,
                              TaskID parentID, TaskID dependencyID)
{
    assert(instancesNum > 0);

    TaskID taskID = mTasksNum++;
    if (taskID >= mMaxTasks)
        return NFE_INVALID_TASK_ID;

    Task& task = mTasks[taskID];
    task.Reset();
    task.mTasksLeft = 1;
    task.mCallback = function;
    task.mParent = parentID;
    task.mInstancesNum =  task.mInstancesLeft = static_cast<uint32>(instancesNum);
    task.mDependency = dependencyID;
    task.mHead = NFE_INVALID_TASK_ID;

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
            {
                mTasks[dependency.mTail].mSibling = taskID;
            }
            else
            {
                assert(dependency.mHead == NFE_INVALID_TASK_ID);
                dependency.mHead = taskID;
            }
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
    if (mTasks[taskID].mTasksLeft == 0)
        return;

    Lock lock(mFinishedTasksMutex);
    while (mTasks[taskID].mTasksLeft > 0)
        mFinishedTasksCV.wait(lock);
}

void ThreadPool::WaitForTasks(TaskID* tasks, size_t tasksNum)
{
    for (size_t i = 0; i < tasksNum; ++i)
    {
        if (mTasks[tasks[i]].mTasksLeft == 0)
            continue;

        Lock lock(mFinishedTasksMutex);
        while (mTasks[tasks[i]].mTasksLeft > 0)
            mFinishedTasksCV.wait(lock);
    }
}

void ThreadPool::WaitForAllTasks()
{
    for (uint32 i = 0; i < mTasksNum; ++i)
    {
        if (mTasks[i].mTasksLeft == 0)
            continue;

        Lock lock(mFinishedTasksMutex);
        while (mTasks[i].mTasksLeft > 0)
            mFinishedTasksCV.wait(lock);
    }

    mTasksNum = 0;
}

} // namespace Common
} // namespace NFE
