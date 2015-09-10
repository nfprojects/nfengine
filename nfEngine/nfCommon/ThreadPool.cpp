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

Task::Task(TaskFunction callback, size_t instancesNum)
    : mState(TaskState::Invalid)
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
                           const TaskID* dependencies, size_t dependenciesNum,
                           bool noWait)
{
    assert(instances > 0);

    TaskID taskPtr = mLastTaskId++; //< generate task id

    Task* task = new Task(function, instances);
    task->ptr = taskPtr;

    bool insertToQueue = false;

    if (dependenciesNum == 0)
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
        assert(dependencies != nullptr);
        task->mRequired = dependenciesNum;

        Lock tasksLock(mTasksMutex);
        mTasks.insert(std::pair<TaskID, Task*>(taskPtr, task));

        Lock depsLock(mDepsQueueMutex);
        for (size_t i = 0; i < dependenciesNum; ++i)
        {
            Task* parentTask = GetTask(dependencies[i]);
            if (parentTask == nullptr) // check if dependent task has not already finished
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

bool ThreadPool::AddDependency(TaskID taskId, const TaskID* dependencies, size_t dependenciesNum)
{
    Task* task = GetTask(taskId);
    return true;
}

Task* ThreadPool::GetTask(TaskID ptr) const
{
    return static_cast<Task*>(ptr);
}

void ThreadPool::WaitForTask(TaskID taskPtr)
{
    Lock lock(mTasksMutex);
    while (GetTask(taskPtr) != nullptr)
        mTasksMutexCV.wait(lock);
}

void ThreadPool::WaitForTasks(TaskID* tasks, size_t tasksNum)
{
    if (tasksNum == 0)
        return;

    assert(tesks != nullptr);
    TaskID tmpTask = Enqueue(TaskFunction(), 1, tasks, tasksNum);

    Lock lock(mTasksMutex);
    while (GetTask(tmpTask) != nullptr)
        mTasksMutexCV.wait(lock);
}

void ThreadPool::WaitForAllTasks()
{
    Lock lock(mTasksMutex);
    while (mTasks.size() > 0)
        mTasksMutexCV.wait(lock);
}

} // namespace Common
} // namespace NFE
