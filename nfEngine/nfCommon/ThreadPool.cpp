/**
 * @file  ThreadPool.cpp
 * @brief Thread pool classes declarations.
 */

#include "stdafx.h"
#include "ThreadPool.hpp"

namespace NFE {
namespace Common {

Task::Task(TaskFunction callback, size_t instancesNum) :
    ptr(0),
    callback(callback),
    instancesNum(instancesNum),
    nextInstance(0),
    instancesLeft(instancesNum),
    required(1)
{
}

void Task::RemoveFromParents()
{
    // assume call under ThreadPool::mDepsQueueMutex lock
    for (Task* parent : parents)
        parent->children.erase(this);
}

//=================================================================================================

WorkerThread::WorkerThread(ThreadPool* pPool, size_t id)
{
    started = true;
    this->id = id;
    thread = std::thread(&ThreadPool::SchedulerCallback, pPool, this);
}

WorkerThread::~WorkerThread()
{
    thread.join();
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
            thread->started = false;

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
    workerThread->started = false;
    mTaskQueueTask.notify_all();
}

size_t ThreadPool::GetThreadsNumber() const
{
	return mThreads.size();
}

void ThreadPool::SetThreadsNumber(size_t newValue)
{
    std::unique_lock<std::mutex> lock(mThreadsMutex);
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

            // wait for new task task
            while (thread->started && mTasksQueue.empty())
                mTaskQueueTask.wait(lock);

            if (!thread->started)
                return;

            // pop a task from the queue
            currTask = mTasksQueue.front();
            instanceId = currTask->nextInstance++;
            if (currTask->nextInstance == currTask->instancesNum)
                mTasksQueue.pop();
        }

        // execute
        if (currTask->callback)
        {
            currTask->callback(instanceId, thread->id);
        }

        // check if task has been completed (last instance just finished its execution)
        if (--currTask->instancesLeft > 0)
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
			for (Task* child : currTask->children)
			{
				child->parents.erase(currTask);
				if (--child->required == 0)
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

TaskPtr ThreadPool::Enqueue(TaskFunction function, size_t instances,
                             const std::vector<TaskPtr>& dependencies, size_t required)
{
    assert(instances > 0);

	TaskPtr taskPtr = mLastTaskId++; //< generate task id

	Task* task = new Task(function, instances);
	task->ptr = taskPtr;
    
    bool insertToQueue = false;

    if (required == 0 || dependencies.empty())
    {
        // tasks with no dependencies
		{
			Lock lock(mTasksMutex);
			mTasks.insert(std::pair<TaskPtr, Task*>(taskPtr, task));
		}
        insertToQueue = true;
    }
    else
    {
		if (required < 0 || required > dependencies.size())
			task->required = dependencies.size();
		else
			task->required = required;

		Lock tasksLock(mTasksMutex);
		mTasks.insert(std::pair<TaskPtr, Task*>(taskPtr, task));

		Lock depsLock(mDepsQueueMutex);
		for (const TaskPtr& parentTaskPtr : dependencies)
		{
			Task* parentTask = GetTask(parentTaskPtr);
			if (parentTask == nullptr)
			{
				task->required--;	
				if (task->required == 0)
				{
					// all dependencies already resolved, move to the queue
                    insertToQueue = true;
                    break;
				}
			}
			else
			{
				task->parents.insert(parentTask);
				parentTask->children.insert(task);
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

Task* ThreadPool::GetTask(const TaskPtr& ptr) const
{
    // assume called in mTasksMutex lock

	auto it = mTasks.find(ptr);
	if (it != mTasks.end())
		return it->second;

	return nullptr;
}

bool ThreadPool::IsTaskFinished(const TaskPtr& taskPtr)
{
    Lock lock(mTasksMutex);
	return GetTask(taskPtr) == nullptr;
}

void ThreadPool::WaitForTask(const TaskPtr& taskPtr)
{
    Lock lock(mTasksMutex);
	while (GetTask(taskPtr) != nullptr)
        mTasksMutexCV.wait(lock);
}

void ThreadPool::WaitForTasks(const std::vector<TaskPtr>& tasks, size_t required)
{
    TaskPtr tmpTask = Enqueue(TaskFunction(), 1, tasks, required);

    Lock lock(mTasksMutex);
    while (GetTask(tmpTask) != nullptr)
        mTasksMutexCV.wait(lock);
}

} // namespace Common
} // namespace NFE
