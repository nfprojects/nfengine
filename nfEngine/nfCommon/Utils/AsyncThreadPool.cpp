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


IAsyncDependency::IAsyncDependency(AsyncThreadPool& pool)
    : mPool(pool)
    , mFullfilled(false)
{
}

void IAsyncDependency::OnDepenencyFulfilled()
{
    const bool fulfilledBefore = mFullfilled.exchange(true);
    NFE_ASSERT(!fulfilledBefore, "Dependency cannot be fulfilled twice");

    for (const AsyncTaskPtr& childTask : mReverseDependencies)
    {
        childTask->Enqueue();
    }

    // clear the list to break the cycle
    mReverseDependencies.Clear();
}


//////////////////////////////////////////////////////////////////////////


AsyncTask::AsyncTask(AsyncThreadPool& pool, const AsyncFuncCallback& callback, const AsyncTaskSetup& setup)
    : IAsyncDependency(pool)
    , mParent(setup.parent)
    , mDependency(setup.dependency)
    , mCallback(callback)
    , mState(State::Pending)
{}

AsyncTask::~AsyncTask()
{
    NFE_ASSERT(mState != State::Executing, "Async tasks has 'executing' state while destruction. This should be not possible");
}

void AsyncTask::Schedule()
{
    const State stateBefore = mState.exchange(State::Sheduled);
    NFE_ASSERT(stateBefore == State::Pending, "Schedule() must be called on a non-scheduled task only");

    if (mDependency)
    {
        ScopedMutexLock lock(GetPool().mTasksQueueMutex);

        // check if the dependency is already fulfilled
        // if so, enqueue immediately
        if (mDependency->IsFulfilled())
        {
            Enqueue();
        }

        // clear the pointer to break the cycle
        mDependency.Reset();
    }
    else
    {
        Enqueue();
    }
}

void AsyncTask::Enqueue()
{
    const State stateBefore = mState.exchange(State::Enqueued);
    NFE_ASSERT(stateBefore == State::Sheduled, "Enqueue() must be called on a scheduled task only");

    // TODO SharedFromThis?
    GetPool().EnqueueTask(this);
}

void AsyncTask::OnExecute()
{
    {
        const State stateBefore = mState.exchange(State::Executing);
        NFE_ASSERT(stateBefore == State::Enqueued, "Execute() must be called on a scheduled task only");
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

    OnFinished();
}

void AsyncTask::OnFinished()
{
    if (--mTasksLeft > 0)
    {
        // there are still some child tasks running
        return;
    }

    // TODO notify the pool about the finished task

    // propagate the update to parent
    if (mParent)
    {
        mParent->OnFinished();

        // cleanup the pointer to break the cycle
        mParent.Reset();
    }

    // notify waiting tasks
    OnDepenencyFulfilled();
}

void AsyncTask::Wait() const
{
    // TODO
}

void AsyncTask::UpdateState(const State newState)
{
    // TODO verify state transition
    mState = newState;
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
        mTaskQueueCV.SignalAll();
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
    AsyncTaskPtr currTask;

    for (;;)
    {
        {
            ScopedMutexLock lock(mTasksQueueMutex);

            // wait for new task
            while (mStarted && mTasksQueue.empty())
            {
                mTaskQueueCV.Wait(lock);
            }

            if (!mStarted)
            {
                return;
            }

            // pop a task from the queue
            currTask = mTasksQueue.front();
            mTasksQueue.pop();
        }

        // execute
        currTask->OnExecute();
    }
}

AsyncTaskPtr AsyncThreadPool::CreateTask(const AsyncFuncCallback& function, const AsyncTaskSetup& setup)
{
    if (setup.parent)
    {
        NFE_ASSERT(setup.parent->GetState() == AsyncTask::State::Pending, "Parent task must be in pending state");
        setup.parent->mTasksLeft++;
    }

    AsyncTaskPtr task = MakeSharedPtr<AsyncTask>(*this, function, setup);

    return task;
}

void AsyncThreadPool::EnqueueTask(const AsyncTaskPtr& task)
{
    ScopedMutexLock lock(mTasksQueueMutex);

    mTasksQueue.push(task);
    mTaskQueueCV.SignalAll();
}


} // namespace Common
} // namespace NFE
