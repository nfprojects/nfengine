/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Async thread pool classes definitions.
 */

#include "PCH.hpp"
#include "AsyncThreadPool.hpp"


namespace NFE {
namespace Common {

AsyncFunc::AsyncFunc(AsyncFuncCallback callback)
    : ptr(0)
    , mCallback(callback)
{}

AsyncThreadPool::AsyncThreadPool()
{
    mLastTaskId = 0;
    mStarted = true;

    // spawn worker threads
    for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i)
        mWorkerThreads.PushBack(std::thread(&AsyncThreadPool::SchedulerCallback, this));
}

AsyncThreadPool::~AsyncThreadPool()
{
    mStarted = false;
    {
        ScopedExclusiveLock<Mutex> lock(mTasksQueueMutex);
        mTaskQueueTask.SignalAll();
    }

    // cleanup worker threads
    for (auto& thread : mWorkerThreads)
        thread.join();
    mWorkerThreads.Clear();

    // cleanup tasks
    for (auto taskPair : mTasks)
        delete taskPair.second;
}

void AsyncThreadPool::SchedulerCallback()
{
    AsyncFunc* currTask = nullptr;

    for (;;)
    {
        {
            ScopedExclusiveLock<Mutex> lock(mTasksQueueMutex);

            // wait for new task
            while (mStarted && mTasksQueue.Empty())
                mTaskQueueTask.Wait(lock);

            if (!mStarted)
                return;

            // pop a task from the queue
            currTask = mTasksQueue.Front();
            mTasksQueue.PopFront();
        }

        // execute
        currTask->mCallback();

        // mark as done and notify waiting threads
        {
            NFE_SCOPED_LOCK(mTasksMutex);
            mTasks.Erase(currTask->ptr);
            mTasksMutexCV.SignalAll();

            // cleanup
            delete currTask;
            currTask = nullptr;
        }
    }
}

AsyncFuncID AsyncThreadPool::Enqueue(AsyncFuncCallback function)
{
    AsyncFuncID taskPtr = mLastTaskId++; //< generate task id
    AsyncFunc* task = new AsyncFunc(function);
    task->ptr = taskPtr;

    {
        NFE_SCOPED_LOCK(mTasksMutex);
        mTasks.Insert(taskPtr, task);
    }

    {
        NFE_SCOPED_LOCK(mTasksQueueMutex);
        mTasksQueue.PushBack(task);
        mTaskQueueTask.SignalAll();
    }

    return taskPtr;
}

AsyncFunc* AsyncThreadPool::GetTask(const AsyncFuncID& taskID) const
{
    // assume called in mTasksMutex lock

    auto it = mTasks.Find(taskID);
    if (it != mTasks.End())
        return it->second;

    return nullptr;
}

bool AsyncThreadPool::IsTaskFinished(const AsyncFuncID& taskID)
{
    NFE_SCOPED_LOCK(mTasksMutex);
    return GetTask(taskID) == nullptr;
}

void AsyncThreadPool::WaitForTask(const AsyncFuncID& taskID)
{
    ScopedExclusiveLock<Mutex> lock(mTasksMutex);
    while (GetTask(taskID) != nullptr)
    {
        mTasksMutexCV.Wait(lock);
    }
}

void AsyncThreadPool::WaitForAllTasks()
{
    ScopedExclusiveLock<Mutex> lock(mTasksMutex);
    while (!mTasks.Empty())
    {
        mTasksMutexCV.Wait(lock);
    }
}

} // namespace Common
} // namespace NFE
