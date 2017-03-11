/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Async thread pool classes definitions.
 */

#include "PCH.hpp"
#include "AsyncThreadPool.hpp"
#include "Logger/Logger.hpp"

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
        mWorkerThreads.emplace_back(std::thread(&AsyncThreadPool::SchedulerCallback, this));
}

AsyncThreadPool::~AsyncThreadPool()
{
    mStarted = false;
    {
        Lock lock(mTasksQueueMutex);
        mTaskQueueTask.notify_all();
    }

    // cleanup worker threads
    for (auto& thread : mWorkerThreads)
        thread.join();
    mWorkerThreads.clear();

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
            Lock lock(mTasksQueueMutex);

            // wait for new task
            while (mStarted && mTasksQueue.empty())
                mTaskQueueTask.wait(lock);

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
            Lock lock(mTasksMutex);
            mTasks.erase(currTask->ptr);
            mTasksMutexCV.notify_all();

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
        Lock lock(mTasksMutex);
        mTasks.insert(std::pair<AsyncFuncID, AsyncFunc*>(taskPtr, task));
    }

    {
        Lock lock(mTasksQueueMutex);
        mTasksQueue.push(task);
        mTaskQueueTask.notify_all();
    }

    return taskPtr;
}

AsyncFunc* AsyncThreadPool::GetTask(const AsyncFuncID& taskID) const
{
    // assume called in mTasksMutex lock

    auto it = mTasks.find(taskID);
    if (it != mTasks.end())
        return it->second;

    return nullptr;
}

bool AsyncThreadPool::IsTaskFinished(const AsyncFuncID& taskID)
{
    Lock lock(mTasksMutex);
    return GetTask(taskID) == nullptr;
}

void AsyncThreadPool::WaitForTask(const AsyncFuncID& taskID)
{
    Lock lock(mTasksMutex);
    mTasksMutexCV.wait(lock, [&] { return GetTask(taskID) == nullptr; });
}

void AsyncThreadPool::WaitForAllTasks()
{
    Lock lock(mTasksMutex);
    mTasksMutexCV.wait(lock, [&] { return mTasks.size() == 0; });
}

} // namespace Common
} // namespace NFE
