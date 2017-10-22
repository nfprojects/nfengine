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
        ScopedMutexLock lock(mTasksQueueMutex);
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
            ScopedMutexLock lock(mTasksQueueMutex);

            // wait for new task
            while (mStarted && mTasksQueue.empty())
                mTaskQueueTask.Wait(lock);

            if (!mStarted)
                return;

            // pop a task from the queue
            currTask = mTasksQueue.front();
            mTasksQueue.pop();
        }

        // execute
        currTask->mCallback();

        // mark as done and notify waiting threads
        {
            ScopedMutexLock lock(mTasksMutex);
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
        ScopedMutexLock lock(mTasksMutex);
        mTasks.Insert(taskPtr, task);
    }

    {
        ScopedMutexLock lock(mTasksQueueMutex);
        mTasksQueue.push(task);
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
    ScopedMutexLock lock(mTasksMutex);
    return GetTask(taskID) == nullptr;
}

void AsyncThreadPool::WaitForTask(const AsyncFuncID& taskID)
{
    ScopedMutexLock lock(mTasksMutex);
    while (GetTask(taskID) != nullptr)
    {
        mTasksMutexCV.Wait(lock);
    }
}

void AsyncThreadPool::WaitForAllTasks()
{
    ScopedMutexLock lock(mTasksMutex);
    while (!mTasks.Empty())
    {
        mTasksMutexCV.Wait(lock);
    }
}

} // namespace Common
} // namespace NFE
