/**
 * @file  ThreadPool.cpp
 * @brief Thread pool classes declarations.
 */

#include "stdafx.hpp"
#include "ThreadPool.hpp"
#include "Logger.hpp"

namespace NFE {
namespace Common {

Task::Task(TaskFunction callback, size_t instancesNum)
    : ptr(0)
    , mCallback(callback)
    , mInstancesNum(instancesNum)
    , mNextInstance(0)
    , mInstancesLeft(instancesNum)
    , mRequired(1)
{
}

void Task::RemoveFromParents()
{
    // assume call under ThreadPool::mDepsQueueMutex lock
    for (Task* parent : mParents)
        parent->mChildren.erase(this);
}

//=================================================================================================

WorkerThread::WorkerThread(ThreadPool* pPool, size_t id)
{
    mStarted = true;
    this->mId = id;
    mThread = std::thread(&ThreadPool::SchedulerCallback, pPool, this);
}

WorkerThread::~WorkerThread()
{
    mThread.join();
}

//=================================================================================================

ThreadPool::ThreadPool()
{
    mLastThreadId = 0;
    mLastTaskId = 0;
    SpawnWorkerThreads(std::thread::hardware_concurrency());
}

ThreadPool::~ThreadPool()
{
    {
        Lock lock(mTasksQueueMutex);

        for (auto thread : mThreads)
            thread->mStarted = false;

        mTaskQueueTask.notify_all();
    }

    // cleanup
    mThreads.clear();
    for (auto taskPair : mTasks)
    {
        delete taskPair.second;
    }
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
    mTaskQueueTask.notify_all();
}

size_t ThreadPool::GetThreadsNumber() const
{
    return mThreads.size();
}

void ThreadPool::SetThreadsNumber(size_t newValue)
{
    Lock lock(mThreadsMutex);
    if (mThreads.size() < newValue)
        SpawnWorkerThreads(newValue - mThreads.size());

    // TODO: implement reducing number of worker threads
}

void ThreadPool::SchedulerCallback(WorkerThread* thread)
{
    size_t instanceId;
    Task* currTask = nullptr;

    for (;;)
    {
        {
            Lock lock(mTasksQueueMutex);

            // wait for new task
            while (thread->mStarted && mTasksQueue.empty())
                mTaskQueueTask.wait(lock);

            if (!thread->mStarted)
                return;

            // pop a task from the queue
            currTask = mTasksQueue.front();
            instanceId = currTask->mNextInstance++;
            if (currTask->mNextInstance == currTask->mInstancesNum)
                mTasksQueue.pop();
        }

        // execute
        if (currTask->mCallback)
        {
            currTask->mCallback(instanceId, thread->mId);
        }

        // check if task has been completed (last instance just finished its execution)
        if (--currTask->mInstancesLeft > 0)
            continue;

        std::vector<Task*> resolved;

        // mark as done and notify waiting threads
        {
            Lock lock(mTasksMutex);
            mTasks.erase(currTask->ptr);
            mTasksMutexCV.notify_all();

            // handle dependencies
            Lock depsLock(mDepsQueueMutex);

            // resolve dependent tasks
            for (Task* child : currTask->mChildren)
            {
                child->mParents.erase(currTask);
                if (--child->mRequired == 0)
                {
                    resolved.push_back(child);
                }
            }

            for (Task* child : resolved)
                child->RemoveFromParents();

            // cleanup
            delete currTask;
            currTask = nullptr;
        }

        if (!resolved.empty())
        {
            Lock queueLock(mTasksQueueMutex);

            // move ready tasks to the queue
            for (Task* child : resolved)
                mTasksQueue.push(child);

            // notify waiting threads only if new task moved to the queue
            mTaskQueueTask.notify_all();
        }
    }
}

TaskID ThreadPool::Enqueue(TaskFunction function, size_t instances,
                           const std::vector<TaskID>& dependencies, size_t required)
{
    assert(instances > 0);

    TaskID taskPtr = mLastTaskId++; //< generate task id

    Task* task = new Task(function, instances);
    task->ptr = taskPtr;

    bool insertToQueue = false;

    if (required == 0 || dependencies.empty())
    {
        // tasks with no dependencies
        {
            Lock lock(mTasksMutex);
            mTasks.insert(std::pair<TaskID, Task*>(taskPtr, task));
        }
        insertToQueue = true;
    }
    else
    {
        if (required < 0 || required > dependencies.size())
            task->mRequired = dependencies.size();
        else
            task->mRequired = required;

        Lock tasksLock(mTasksMutex);
        mTasks.insert(std::pair<TaskID, Task*>(taskPtr, task));

        Lock depsLock(mDepsQueueMutex);
        for (const TaskID& parentTaskPtr : dependencies)
        {
            Task* parentTask = GetTask(parentTaskPtr);
            if (parentTask == nullptr)
            {
                task->mRequired--;
                if (task->mRequired == 0)
                {
                    // all dependencies already resolved, move to the queue
                    insertToQueue = true;
                    break;
                }
            }
            else
            {
                task->mParents.insert(parentTask);
                parentTask->mChildren.insert(task);
            }
        }
    }

    if (insertToQueue)
    {
        Lock lock(mTasksQueueMutex);
        mTasksQueue.push(task);
        mTaskQueueTask.notify_all();
    }

    return taskPtr;
}

Task* ThreadPool::GetTask(const TaskID& ptr) const
{
    // assume called in mTasksMutex lock

    auto it = mTasks.find(ptr);
    if (it != mTasks.end())
        return it->second;

    return nullptr;
}

bool ThreadPool::IsTaskFinished(const TaskID& taskPtr)
{
    Lock lock(mTasksMutex);
    return GetTask(taskPtr) == nullptr;
}

void ThreadPool::WaitForTask(const TaskID& taskPtr)
{
    Lock lock(mTasksMutex);
    while (GetTask(taskPtr) != nullptr)
        mTasksMutexCV.wait(lock);
}

void ThreadPool::WaitForTasks(const std::vector<TaskID>& tasks, size_t required)
{
    TaskID tmpTask = Enqueue(TaskFunction(), 1, tasks, required);

    Lock lock(mTasksMutex);
    while (GetTask(tmpTask) != nullptr)
        mTasksMutexCV.wait(lock);
}

} // namespace Common
} // namespace NFE
