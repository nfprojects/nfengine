/**
* @brief Simple unit tests for ThreadPool class.
*/

#include "PCH.hpp"
#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Utils/ThreadPoolTask.hpp"
#include "nfCommon/Utils/Latch.hpp"
#include "nfCommon/Utils/Waitable.hpp"
#include "nfCommon/Utils/TaskBuilder.hpp"
#include "nfCommon/Utils/ParallelAlgorithms.hpp"
#include "nfCommon/System/Timer.hpp"
#include "nfCommon/Math/Random.hpp"

using namespace NFE;
using namespace NFE::Common;

TEST(ThreadPoolSimple, EmptyTask_NoWaiting)
{
    TaskDesc desc;
    desc.function = [](const TaskContext&) {};

    ThreadPool::GetInstance().CreateAndDispatchTask(desc);

    // Note:
    // We can't do any assumptions about the task here.
    // It could be even not executed at all.
}

TEST(ThreadPoolSimple, EmptyTasks_NoWaiting)
{
    TaskDesc desc;
    desc.function = [] (const TaskContext&) {};

    for (uint32 i = 0; i < 1000; ++i)
    {
        ThreadPool::GetInstance().CreateAndDispatchTask(desc);
    }

    // Note:
    // We can't do any assumptions about the task here.
    // It could be even not executed at all.
}

TEST(ThreadPoolSimple, EmptyWaitableTask_Waiting)
{
    std::atomic<uint32> counter(0);

    Latch latch;

    Waitable waitable;
    ASSERT_FALSE(waitable.IsFinished());

    TaskDesc desc;
    desc.function = [&latch, &counter](const TaskContext&)
    {
        latch.Wait();
        counter++;
    };
    desc.waitable = &waitable;

    ThreadPool::GetInstance().CreateAndDispatchTask(desc);

    // the task is blocked, it couldn't get finished
    ASSERT_FALSE(waitable.IsFinished());

    // unblock and wait
    latch.Set();
    waitable.Wait();
    ASSERT_TRUE(waitable.IsFinished());

    // waiting for the task for the second time should do nothing
    waitable.Wait();
    ASSERT_TRUE(waitable.IsFinished());

    // make sure the task was executed only once
    ASSERT_EQ(1u, counter.load());
}

TEST(ThreadPoolSimple, SimpleDependency)
{
    const uint32 numIterations = 1000;

    for (uint32 i = 0; i < numIterations; ++i)
    {
        Waitable waitable;

        std::atomic<uint32> counterA(0);
        std::atomic<uint32> counterB(0);
        std::atomic<bool> dispatchedA(false);
        std::atomic<bool> dispatchedB(false);

        TaskID taskA, taskB;

        {
            TaskDesc desc;
            desc.debugName = "A";
            desc.function = [&] (const TaskContext&)
            {
                ASSERT_TRUE(dispatchedA.load());
                ASSERT_EQ(0u, counterA.load());
                ASSERT_EQ(0u, counterB.load());
                counterA++;
            };
            taskA = ThreadPool::GetInstance().CreateTask(desc);
        }

        {
            TaskDesc desc;
            desc.debugName = "B";
            desc.function = [&] (const TaskContext&)
            {
                ASSERT_TRUE(dispatchedB.load());
                ASSERT_EQ(1u, counterA.load());
                ASSERT_EQ(0u, counterB.load());
                counterB++;
            };
            desc.dependency = taskA;
            desc.waitable = &waitable;
            taskB = ThreadPool::GetInstance().CreateTask(desc);
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(10));

        dispatchedA = true;
        ThreadPool::GetInstance().DispatchTask(taskA);

        //std::this_thread::sleep_for(std::chrono::milliseconds(10));

        dispatchedB = true;
        ThreadPool::GetInstance().DispatchTask(taskB);

        // unblock and wait
        waitable.Wait();
        ASSERT_TRUE(waitable.IsFinished());

        ASSERT_EQ(1u, counterA.load());
        ASSERT_EQ(1u, counterB.load());
    }
}

// Spawn 1000 tasks.
TEST(ThreadPoolSimple, ThousandTasks)
{
    const uint32 numTasks = 1000;
    std::atomic<uint32> counter(0);
    {
        std::vector<Waitable> waitables{ numTasks };

        auto emptyTaskFunc = [&counter](const TaskContext&) { counter++; };

        for (uint32 i = 0; i < numTasks; ++i)
        {
            TaskDesc desc;
            desc.function = emptyTaskFunc;
            desc.waitable = &waitables[i];
            ThreadPool::GetInstance().CreateAndDispatchTask(desc);
        }

        for (uint32 i = 0; i < numTasks; ++i)
        {
            waitables[i].Wait();
        }
    }

    EXPECT_EQ(numTasks, counter);
}

// Spawn 1000 child tasks
TEST(ThreadPoolSimple, ChildTasks)
{
    const int numTasks = 1000;
    std::atomic<int> counter(0);
    {
        auto emptyTaskFunc = [&counter](const TaskContext&) { counter++; };

        Waitable waitable;

        TaskDesc rootTaskDesc;
        rootTaskDesc.waitable = &waitable;
        TaskID rootTask = ThreadPool::GetInstance().CreateTask(rootTaskDesc);

        for (int i = 0; i < numTasks; ++i)
        {
            TaskDesc desc;
            desc.function = emptyTaskFunc;
            desc.parent = rootTask;
            ThreadPool::GetInstance().CreateAndDispatchTask(desc);
        }
        
        ThreadPool::GetInstance().DispatchTask(rootTask);

        waitable.Wait();
    }

    EXPECT_EQ(numTasks, counter);
}

// Create long dependency chain: A -> B -> C -> D ...
// Verify execution order.
TEST(ThreadPoolSimple, DependencyChain)
{
    const int chainLen = 4000; // chain length

    Latch latch;
    std::atomic<uint32> counters[chainLen];

    auto func = [&](int task)
    {
        if (task == 0)
        {
            latch.Wait();
        }
        else
        {
            // verify previous task was finished
            EXPECT_EQ(1u, counters[task - 1]) << "Task #" << task;
        }

        counters[task]++;
    };

    for (int i = 0; i < chainLen; ++i)
        counters[i] = 0;

    Waitable waitable;

    TaskDesc desc;
    desc.function = [&func](const TaskContext&) { func(0); };
    TaskID firstTask = ThreadPool::GetInstance().CreateTask(desc);
    TaskID prevTask = firstTask;
    for (int i = 1; i < chainLen; ++i)
    {
        desc.function = [&func, i](const TaskContext&) { func(i); };
        desc.dependency = prevTask;
        desc.waitable = (i == chainLen - 1) ? &waitable : nullptr;
        prevTask = ThreadPool::GetInstance().CreateTask(desc);
        ThreadPool::GetInstance().DispatchTask(prevTask);
    }
    ThreadPool::GetInstance().DispatchTask(firstTask);

    // verify counters (nothing should be executed yet)
    for (int i = 0; i < chainLen; ++i)
    {
        EXPECT_EQ(0u, counters[i]) << "Task #" << i;
    }

    latch.Set();
    waitable.Wait();

    // verify counters (everything should be finished)
    for (int i = 0; i < chainLen; ++i)
    {
        EXPECT_EQ(1u, counters[i]) << "Task #" << i;
    }
}

// Spawn a child task inside another recursively
TEST(ThreadPoolSimple, EnqueueInsideTaskRecursive)
{
    const int numTasks = 4000;

    std::atomic<int32> count(0);

    TaskFunction taskFunc = [&](const TaskContext& context)
    {
        if (++count < numTasks)
        {
            TaskDesc desc;
            desc.function = taskFunc;
            desc.parent = context.taskId;
            ThreadPool::GetInstance().CreateAndDispatchTask(desc);
        }
    };

    Waitable waitable;

    /// spawn first task
    TaskDesc desc;
    desc.function = taskFunc;
    desc.waitable = &waitable;
    ThreadPool::GetInstance().CreateAndDispatchTask(desc);

    waitable.Wait();
    ASSERT_EQ(numTasks, count);
}

// Spawn 2 child tasks inside another recursively (binary-tree-like structure is created)
TEST(ThreadPoolSimple, EnqueueChildRecursive)
{
    using namespace std::placeholders;
    const uint32 maxDepth = 14;
    const uint32 children = 2;

    std::atomic<uint32> count = 0u;

    std::function<void(const TaskContext&, uint32)> func;
    func = [&](const TaskContext& context, uint32 depth)
    {
        count++;
        if (depth >= maxDepth)
            return;

        for (uint32 i = 0; i < children; ++i)
        {
            TaskDesc desc;
            desc.function = [&func, depth](const TaskContext& context) { func(context, depth + 1); };
            desc.parent = context.taskId;
            ThreadPool::GetInstance().CreateAndDispatchTask(desc);
        }
    };

    Waitable waitable;

    // spawn root task
    TaskDesc desc;
    desc.function = [&func](const TaskContext& context) { func(context, 0); };
    desc.waitable = &waitable;
    ThreadPool::GetInstance().CreateAndDispatchTask(desc);

    waitable.Wait();
    EXPECT_EQ((1u << (maxDepth + 1)) - 1u, count.load());
}

// Spawn 2 child tasks inside another recursively (binary-tree-like structure is created)
TEST(ThreadPoolSimple, ParallelSort)
{
    uint32 numElements = 1000000;

    DynArray<uint32> array;
    array.Resize(numElements);

    Math::Random random;
    for (uint32 i = 0; i < numElements; ++i)
    {
        array[i] = random.GetInt();
    }

    DynArray<uint32> reference = array;

    double regularSortTime = 0.0;
    {
        Timer timer;
        std::sort(reference.Begin(), reference.End());
        regularSortTime = timer.Stop();
        NFE_LOG_INFO("std::sort: %.3f ms", 1000.0 * regularSortTime);
    }

    {
        Timer timer;
        Waitable waitable;
        {
            TaskBuilder builder(waitable);
            ParallelSort(array.Begin(), array.End(), builder);
        }
        waitable.Wait();
        double parallelSortTime = timer.Stop();
        NFE_LOG_INFO("ParallelSort: %.3f ms (%.2fx)", 1000.0 * parallelSortTime, regularSortTime/parallelSortTime);
    }

    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_EQ(reference[i], array[i]) << "i=" << std::to_string(i);
    }
}
