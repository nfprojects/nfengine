/**
    NFEngine project

    \file   ThreadPool.cpp
    \brief  Thread pool classes declarations.
*/

#include "stdafx.hpp"
#include "ThreadPool.hpp"
#include "Logger.hpp"

namespace NFE {
namespace Common {

const int gMaxThreadPoolThreads = 64;

void* ExecutorThread::operator new(size_t size)
{
    return _aligned_malloc(size, 64);
}

void ExecutorThread::operator delete(void* ptr)
{
    _aligned_free(ptr);
}

void ThreadPool::SchedulerThreadCallback(ExecutorThread* pThreadDesc)
{
    ThreadPool* pPool = pThreadDesc->pThreadPool;

    TaskCallback pCallback = nullptr;
    void* pUserData = nullptr;
    uint32 instance = 0;
    TaskID nextTaskID = 0;

    for (;;)
    {
        //gather a new task from the queue
        {
            LockType lock(pPool->mTaskQueueMutex);

            // wait for an event
            while (pThreadDesc->pThreadPool->mStarted && pPool->mTaskQueue.empty())
                pPool->mTaskQueueEvent.wait(lock);

            // thread pool was stopped, exit
            if (!pThreadDesc->pThreadPool->mStarted)
                return;

            //find next not done task
            Task& nextTask = pPool->mTaskQueue.front();
            instance = nextTask.instancesCount - (nextTask.instancesLeft--);
            pCallback = nextTask.pCallback;
            pUserData = nextTask.pUserData;
            nextTaskID = nextTask.id;

            if (nextTask.instancesLeft == 0)
            {
                pPool->mTaskQueue.pop();
            }

            pThreadDesc->busy = true;
            pThreadDesc->currTask = nextTaskID;
        }

        //execute task
        if (pCallback)
        {
            try
            {
                pCallback((void*)pUserData, instance, pThreadDesc->id);
            }
            catch (std::exception& ex)
            {
                LOG_ERROR("Unhandled exception in task callback: %s", ex.what());
            }
            catch (...)
            {
                LOG_ERROR("Unknown exception in task callback");
            }
        }

        //inform pool that task instance has been done
        std::unique_lock<std::mutex> lock(pPool->mTaskFinishedMutex);
        pThreadDesc->busy = false;
        pPool->mTaskFinishedEvent.notify_all();
    }
}

ThreadPool::ThreadPool()
{
    mStarted = false;
}

ThreadPool::~ThreadPool()
{
    Release();
}

int ThreadPool::Init(size_t threadsCount)
{
    if (mStarted) return 1;
    mStarted.exchange(true);

    if (threadsCount == 0)
    {
        threadsCount = std::thread::hardware_concurrency();
        if (threadsCount < 1) threadsCount = 1;
    }
    else
    {
        if (threadsCount < 1)
            threadsCount = 1;
        else if (threadsCount > gMaxThreadPoolThreads)
            threadsCount = gMaxThreadPoolThreads;
    }

    //create threads
    for (size_t i = 0; i < threadsCount; i++)
    {
        ExecutorThread* pThread = new ExecutorThread(this, i);
        mThreads.push_back(pThread);

        try
        {
            pThread->thread = std::thread(SchedulerThreadCallback, pThread);
        }
        catch (...)
        {
            delete pThread;
            break;
        }
    }

    mLastTaskId = 0;
    return 0;
}

void ThreadPool::Release()
{
    if (!mStarted) return;

    {
        std::unique_lock<std::mutex> lock(mTaskQueueMutex);
        mStarted.exchange(false);
        mTaskQueueEvent.notify_all();
    }

    //wait for all threads & destroy them
    for (auto& thread : mThreads)
    {
        thread->thread.join();
        delete thread;
    }
}

const size_t ThreadPool::GetThreadsCount() const
{
    return mThreads.size();
}

TaskID ThreadPool::AddTask(TaskCallback pCallback, void* pUserData, uint32 instancesCount)
{
    if (!mStarted) return 0;
    if (instancesCount == 0) return 0;

    //fill up task structure
    Task task(mLastTaskId++, pCallback, pUserData, instancesCount);

    //add task to the queue
    {
        std::unique_lock<std::mutex> lock(mTaskQueueMutex);
        mTaskQueue.push(task);

        //inform threads that a new task is waiting
        mTaskQueueEvent.notify_all();
    }

    return task.id;
}

bool ThreadPool::IsTaskDone(const TaskID task) const
{
    if (!mStarted) return false;

    bool done = true;

    LockType ulock(mTaskQueueMutex);

    //find task in the queue
    if (!mTaskQueue.empty())
    {
        const Task& first = mTaskQueue.front();
        const Task& last = mTaskQueue.back();
        done = (task < first.id) || (task > last.id);
    }

    //check if a thread is processing the task
    for (auto thread : mThreads)
    {
        if (thread->busy && thread->currTask == task)
        {
            done = false;
            break;
        }
    }

    return done;
}

bool ThreadPool::AreAllTasksDone() const
{
    if (!mStarted) return false;

    bool done = true;
    LockType ulock(mTaskQueueMutex);

    //find task in the queue
    if (!mTaskQueue.empty())
    {
        //check if a thread is processing the task
        for (auto thread : mThreads)
        {
            if (thread->busy)
            {
                done = false;
                break;
            }
        }
    }
    return done;
}

int ThreadPool::WaitForTask(const TaskID task)
{
    if (!mStarted) return 1;

    // deadlock protection
    for (const auto& thread : mThreads)
        if (thread->thread.get_id() == std::this_thread::get_id())
            return 1;

    {
        LockType lk(mTaskFinishedMutex);

        //wait for signal
        while (!IsTaskDone(task))
            mTaskFinishedEvent.wait(lk);
    }

    return 0;
}

int ThreadPool::WaitForAllTasks()
{
    if (!mStarted) return 1;

    // deadlock protection
    for (const auto& thread : mThreads)
        if (thread->thread.get_id() == std::this_thread::get_id())
            return 1;

    {
        LockType lk(mTaskFinishedMutex);

        //wait for signal
        while (!AreAllTasksDone())
            mTaskFinishedEvent.wait(lk);
    }

    return 0;
}

size_t ThreadPool::GetLoad() const
{
    if (!mStarted) return 0;
    if (!mTaskQueue.empty()) return mThreads.size();

    uint32 load = 0;
    for (auto thread : mThreads)
    {
        if (thread->busy)
            load++;
    }

    return load;
}

} // namespace Common
} // namespace NFE