/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance tests for ThreadPool class.
 */

#include "PCH.hpp"
#include "Engine/Common/Utils/ThreadPool.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"
#include "Engine/Common/Utils/Waitable.hpp"
#include "Engine/Common/Utils/Latch.hpp"
#include "Engine/Common/System/Timer.hpp"

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

NFE_FORCE_INLINE static uint32 XorShift(uint32 x)
{
    x ^= x << 13u;
    x ^= x >> 17u;
    x ^= x << 5u;
    return x;
}

TEST(ThreadPool, ParallelFor)
{
    ThreadPool& tp = ThreadPool::GetInstance();

    const uint32 numElements = 16 * 1024 * 1024;

    Common::DynArray<uint32> elements;
    elements.Resize(numElements);

    // fill up the array and compute reference hash
    uint32 refHash = 0;
    uint32 seed = 12345;
    for (uint32 i = 0; i < numElements; ++i)
    {
        elements[i] = seed;
        seed = XorShift(seed);
        refHash ^= elements[i];
    }

    struct NFE_ALIGN(64) ThreadData
    {
        uint32 hash = 0;
    };
    Common::DynArray<ThreadData> threadData;
    threadData.Resize(tp.GetNumThreads());

    Timer timer;
    Waitable waitable;
    timer.Start();
    {
        TaskBuilder taskBuilder(waitable);
        taskBuilder.ParallelFor("ParallelFor", numElements, [&] (const TaskContext& ctx, const uint32 i)
        {
            threadData[ctx.threadId].hash ^= elements[i];
        });
    }
    waitable.Wait();
    double waitTime = 1000.0 * timer.Stop();

    uint32 hash = 0;
    for (uint32 i = 0; i < tp.GetNumThreads(); ++i)
    {
        hash ^= threadData[i].hash;
    }

    NFE_LOG_INFO("Ref hash:      %x", refHash);
    NFE_LOG_INFO("Computed hash: %x", hash);
    NFE_LOG_INFO("Parallel for: %.3f ms", waitTime);
}
