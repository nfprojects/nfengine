/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance tests for ThreadPool class.
 */

#include "PCH.hpp"
#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/System/Timer.hpp"
#include "nfCommon/Utils/Latch.hpp"


using namespace NFE::Common;

TEST(ThreadPool, SpawnTasks)
{
    Timer timer;

    std::cout << "Tasks num | Enqueue time | Wait time" << std::endl;

    for (int numTasks = 2; numTasks < 100000; numTasks *= 2)
    {
        ThreadPool tp;
        std::vector<TaskID> tasks;
        tasks.reserve(numTasks);

        TaskFunction taskFunc = [&](const TaskContext&) {};

        // eneuque time measure
        timer.Start();
        for (int i = 0; i < numTasks; ++i)
            tasks.push_back(tp.CreateTask(taskFunc));
        double enqueueTime = 1000.0 * timer.Stop();

        // wait for tasks time measure
        timer.Start();
        tp.WaitForTasks(tasks.data(), tasks.size());
        double waitTime = 1000.0 * timer.Stop();

        std::cout << std::setprecision(4) << std::left
                  << std::setw(9) << numTasks << " | "
                  << std::setw(12) << enqueueTime << " | "
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
        TaskID grouppingTask = tp.CreateTask(taskFunc, NFE_INVALID_TASK_ID, dependencyTask);

        // eneuque time measure
        timer.Start();
        for (int i = 0; i < numTasks; ++i)
            tasks.push_back(tp.CreateTask(taskFunc, grouppingTask, dependencyTask));
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
