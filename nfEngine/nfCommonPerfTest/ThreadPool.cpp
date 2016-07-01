/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance tests for ThreadPool class.
 */

#include "PCH.hpp"
#include "nfCommon/ThreadPool.hpp"
#include "nfCommon/Timer.hpp"
#include "nfCommon/Latch.hpp"


using namespace NFE::Common;

TEST(ThreadPool, SpawnTasks)
{
    Timer timer;

    std::cout << "Tasks num | Enqueue time | Wait time" << std::endl;

    for (int numTasks = 2; numTasks < 100000; numTasks *= 2)
    {
        Latch latch;
        ThreadPool tp;
        std::vector<TaskID> tasks;
        tasks.reserve(numTasks);

        TaskFunction lockTask = [&](const TaskContext& /* context */)
        {
            latch.Wait();
        };

        TaskFunction taskFunc = [&](const TaskContext& /* context */) {};

        // block worker threads
        tp.CreateTask(lockTask, tp.GetThreadsNumber());

        // eneuque time measure
        timer.Start();
        for (int i = 0; i < numTasks; ++i)
            tasks.push_back(tp.CreateTask(taskFunc, 1, NFE_INVALID_TASK_ID, NFE_INVALID_TASK_ID, true));
        double enqueueTime = 1000.0 * timer.Stop();

        // unlock worker threads
        timer.Start();
        latch.Set();

        // wait for tasks time measure
        tp.WaitForTasks(tasks.data(), tasks.size());
        double waitTime = 1000.0 * timer.Stop();

        std::cout << std::setprecision(4) << std::left
                  << std::setw(9) << numTasks << " | "
                  << std::setw(12) << enqueueTime << " | "
                  << waitTime << std::endl;
    }
}

TEST(ThreadPool, Instances)
{
    Timer timer;

    std::cout << "Instances | Wait time" << std::endl;

    for (int numTasks = 2; numTasks < 1000000; numTasks *= 2)
    {
        Latch latch;
        ThreadPool tp;

        TaskFunction lockTask = [&](const TaskContext& /* context */)
        {
            latch.Wait();
        };

        TaskFunction taskFunc = [&](const TaskContext& /* context */) {};

        // block worker threads
        tp.CreateTask(lockTask, tp.GetThreadsNumber());
        TaskID task = tp.CreateTask(taskFunc, numTasks, NFE_INVALID_TASK_ID, NFE_INVALID_TASK_ID, true);

        // unlock worker threads
        timer.Start();
        latch.Set();
        // wait for tasks time measure
        tp.WaitForTask(task);
        double waitTime = 1000.0 * timer.Stop();

        std::cout << std::setprecision(4) << std::left
                  << std::setw(9) << numTasks << " | "
                  << waitTime << std::endl;
    }
}

TEST(ThreadPool, Dependencies)
{
    Timer timer;

    std::cout << "Tasks num | Enqueue time | Wait time" << std::endl;

    for (int numTasks = 2; numTasks < 100000; numTasks *= 2)
    {
        Latch latch;
        ThreadPool tp;
        std::vector<TaskID> tasks;
        tasks.reserve(numTasks);

        TaskFunction lockTask = [&](const TaskContext& /* context */)
        {
            latch.Wait();
        };

        TaskFunction taskFunc = [&](const TaskContext& /* context */) {};

        // blocking task
        TaskID dependencyTask = tp.CreateTask(lockTask);
        TaskID grouppingTask = tp.CreateTask(taskFunc, 1, NFE_INVALID_TASK_ID, dependencyTask, true);

        // eneuque time measure
        timer.Start();
        for (int i = 0; i < numTasks; ++i)
            tasks.push_back(tp.CreateTask(taskFunc, 1, grouppingTask, dependencyTask));
        double enqueueTime = 1000.0 * timer.Stop();

        // unlock worker threads
        timer.Start();
        latch.Set();

        // wait for tasks time measure
        tp.WaitForTask(grouppingTask);
        double waitTime = 1000.0 * timer.Stop();

        std::cout << std::setprecision(4) << std::left
            << std::setw(9) << numTasks << " | "
            << std::setw(12) << enqueueTime << " | "
            << waitTime << std::endl;
    }
}
