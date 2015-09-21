/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance tests for ThreadPool class.
 */

#include "PCH.hpp"
#include "../nfCommon/ThreadPool.hpp"
#include "../nfCommon/Timer.hpp"
#include "../nfCommon/Latch.hpp"


using namespace NFE::Common;

TEST(ThreadPool, SpawnTasks)
{
    Timer timer;

    std::cout << "Tasks num | Enqueue time | Wait time" << std::endl;

    for (int numTasks = 2; numTasks < 10000; numTasks *= 2)
    {
        Latch latch;
        ThreadPool tp;
        std::vector<TaskID> tasks;
        tasks.reserve(numTasks);

        TaskFunction lockTask = [&](size_t, size_t)
        {
            latch.Wait();
        };

        TaskFunction taskFunc = [&](size_t, size_t) {};

        // block worker threads
        tp.CreateTask(lockTask, tp.GetThreadsNumber());

        // eneuque time measure
        timer.Start();
        for (int i = 0; i < numTasks; ++i)
            tasks.push_back(tp.CreateTask(taskFunc));
        double enqueueTime = 1000.0 * timer.Stop();

        // unlock worker threads
        latch.Set();

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
