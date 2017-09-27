/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Async thread pool classes definitions.
 */

#include "PCH.hpp"
#include "AsyncThreadPool.hpp"
#include "ScopedLock.hpp"
#include "Logger/Logger.hpp"

namespace NFE {
namespace Common {


AsyncTask::AsyncTask(AsyncThreadPool& pool, const AsyncFuncCallback& callback)
    : mPool(pool)
    , mCallback(callback)
    , mState(State::Pending)
{}

void AsyncTask::AddDependency(const AsyncDependencyPtr& dep)
{
    NFE_ASSERT(mState == State::Pending, "Task dependencies must be set for non-scheduled task");

    // TODO synchronize access
    mDependencies.PushBack(dep);
}

void AsyncTask::AddDependencies(const ArrayView<const AsyncDependencyPtr> deps)
{
    NFE_ASSERT(mState == State::Pending, "Task dependencies must be set for non-scheduled task");

    // TODO synchronize access
    mDependencies.PushBackArray(deps);
}

void AsyncTask::Schedule()
{
    // TODO CAS
}

void AsyncTask::Execute()
{
    {
        const State stateBefore = mState.exchange(State::Executing);
        NFE_ASSERT(stateBefore == State::Sheduled, "Execute() must be called on a scheduled task only");
    }

    if (mCallback)
    {
        const AsyncTaskContext context =
        {
            *this,
            0,      // TODO thread ID
        };

        mCallback(context);
    }

    {
        const State stateBefore = mState.exchange(State::Executed);
        NFE_ASSERT(stateBefore == State::Executing, "Task state has been changed externally during execution");
    }

    // notify waiting tasks
    OnDepenencyFulfulled();
}

void AsyncTask::Wait() const
{

}



//////////////////////////////////////////////////////////////////////////

AsyncThreadPool::AsyncThreadPool()
{
    mStarted = true;

    // spawn worker threads
    for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
        mWorkerThreads.PushBack(std::thread(&AsyncThreadPool::SchedulerCallback, this));
    }
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
    {
        thread.join();
    }
    mWorkerThreads.Clear();
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
            mTasks.erase(currTask->ptr);
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
        mTasks.insert(std::pair<AsyncFuncID, AsyncFunc*>(taskPtr, task));
    }

    {
        ScopedMutexLock lock(mTasksQueueMutex);
        mTasksQueue.push(task);
        mTaskQueueTask.SignalAll();
    }

    return taskPtr;
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
    while (!mTasks.empty())
    {
        mTasksMutexCV.Wait(lock);
    }
}

} // namespace Common
} // namespace NFE
