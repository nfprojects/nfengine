/**
* @brief Stress tests for ThreadPool class.
*/

#include "stdafx.h"
#include "../nfCommon/ThreadPool.hpp"
#include "../nfCommon/Timer.h"
#include "Common.hpp"


using namespace NFE::Common;

TEST(ThreadPoolPerformance, SpawnTasks)
{
    Timer timer;

    std::cout << "Tasks num | Enqueue time | Wait time" << std::endl;

    for (int numTasks = 2; numTasks < 100000; numTasks *= 2)
    {
        Latch latch;
        ThreadPool tp;
        std::vector<TaskPtr> tasks;
        tasks.reserve(numTasks);

        TaskFunction lockTask = [&](size_t, size_t)
        {
            latch.Wait();
        };

        TaskFunction taskFunc = [&](size_t, size_t) {};

        // block worker threads
        tp.Enqueue(lockTask, tp.GetThreadsNumber());

        // eneuque time measure
        timer.Start();
        for (int i = 0; i < numTasks; ++i)
            tasks.push_back(tp.Enqueue(taskFunc, 1));
        double enqueueTime = 1000.0 * timer.Stop();

        // unlock worker threads
        latch.Set();

        // wait for tasks time measure
        timer.Start();
        tp.WaitForTasks(tasks);
        double waitTime = 1000.0 * timer.Stop();

        std::cout << std::setprecision(4) << std::left
            << std::setw(9) << numTasks << " | "
            << std::setw(12) << enqueueTime << " | "
            << waitTime << std::endl;
    }
}
