/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance tests for ThreadPool class.
 */

#include "PCH.hpp"
#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Utils/Waitable.hpp"
#include "nfCommon/Utils/Latch.hpp"
#include "nfCommon/System/Timer.hpp"

using namespace NFE;
using namespace NFE::Common;

TEST(ThreadPool, SpawnTasks)
{
    Timer timer;

    std::cout << "Tasks num | Enqueue time | Wait time" << std::endl;

    ThreadPool& tp = ThreadPool::GetInstance();

    for (uint32 numTasks = 2; numTasks < ThreadPool::TasksCapacity - 2; numTasks *= 2)
    {
        Waitable waitable;

        std::atomic<uint32> counter = 0;

        // block worker threads
        TaskID blockingTask;
        {
            TaskDesc desc;
            blockingTask = tp.CreateTask(desc);
        }

        // block worker threads
        TaskID waitableTask;
        {
            TaskDesc desc;
            desc.waitable = &waitable;
            waitableTask = tp.CreateTask(desc);
        }

        // eneuque time measure
        timer.Start();
        for (uint32 i = 0; i < numTasks; ++i)
        {
            TaskDesc desc;
            desc.function = [&counter] (const TaskContext& /* context */) { counter++; };
            desc.dependency = blockingTask;
            desc.parent = waitableTask;
            tp.CreateAndDispatchTask(desc);
        }
        double enqueueTime = 1000.0 * timer.Stop();

        // unlock worker threads
        timer.Start();
        tp.DispatchTask(blockingTask);
        tp.DispatchTask(waitableTask);

        // wait for tasks time measure
        waitable.Wait();
        double waitTime = 1000.0 * timer.Stop();

        ASSERT_EQ(numTasks, counter.load());

        std::cout << std::setprecision(4) << std::left
                  << std::setw(9) << numTasks << " | "
                  << std::setw(12) << enqueueTime << " | "
                  << waitTime << std::endl;
    }
}
