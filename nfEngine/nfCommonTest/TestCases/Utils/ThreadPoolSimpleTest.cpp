/**
* @brief Simple unit tests for ThreadPool class.
*/

#include "PCH.hpp"
#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Utils/Latch.hpp"

using namespace NFE;
using namespace NFE::Common;

TEST(ThreadPoolSimple, ConstructorDestructor)
{
    ThreadPool tp;
}

// Spawn an empty task, no waiting.
TEST(ThreadPoolSimple, EmptyTask)
{
    ThreadPool tp;
    auto emptyTaskFunc = [](const TaskContext& /* context */) {};
    tp.CreateTask(emptyTaskFunc);
}

// Destroy a pool while executing a task
TEST(ThreadPoolSimple, DestroyWhileExecuting)
{
    std::unique_ptr<ThreadPool> tp(new ThreadPool);
    auto taskFunc = [](const TaskContext& /* context */)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    };

    TaskID task = 0;
    ASSERT_NE(NFE_INVALID_TASK_ID, task = tp->CreateTask(taskFunc));
    ASSERT_NE(NFE_INVALID_TASK_ID, tp->CreateTask(taskFunc, NFE_INVALID_TASK_ID, task));
    tp.reset();
}

// Spawn small and long task, wait in various orders.
TEST(ThreadPoolSimple, Wait)
{
    ThreadPool tp;
    TaskFunction smallTaskFunc = [](const TaskContext& /* context */)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    };
    TaskFunction longTaskFunc = [](const TaskContext& /* context */)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    };

    TaskID taskA, taskB, taskC, taskD;

    // wait for already executed
    ASSERT_NE(NFE_INVALID_TASK_ID, taskA = tp.CreateTask(smallTaskFunc));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    tp.WaitForTask(taskA);

    // wait for being executed
    ASSERT_NE(NFE_INVALID_TASK_ID, taskB = tp.CreateTask(longTaskFunc));
    tp.WaitForTask(taskB);

    // wait for queued
    ASSERT_NE(NFE_INVALID_TASK_ID, taskC = tp.CreateTask(longTaskFunc));
    ASSERT_NE(NFE_INVALID_TASK_ID, taskD = tp.CreateTask(longTaskFunc));
    tp.WaitForTask(taskD);
    tp.WaitForTask(taskC);
}

// Spawn a task inside another
TEST(ThreadPoolSimple, EnqueueInsideTask)
{
    ThreadPool tp;
    TaskID task;

    for (int i = 0; i < 10000; ++i)
    {
        std::atomic<int> condition(0);
        Latch latch;

        TaskFunction taskFuncB = [&](const TaskContext&)
        {
            EXPECT_EQ(1, condition);
            latch.Set();
        };

        TaskFunction taskFuncA = [&](const TaskContext& context)
        {
            condition = 1;
            task = tp.CreateTask(taskFuncB, NFE_INVALID_TASK_ID, context.taskId);
            ASSERT_NE(NFE_INVALID_TASK_ID, task);
        };

        tp.CreateTask(taskFuncA);

        latch.Wait();
        tp.WaitForTask(task);
    }
}

// Verify thread id passed in task callback parameters
TEST(ThreadPoolSimple, ThreadId)
{
    ThreadPool tp;
    uint32 workerThreads = tp.GetThreadsNumber();
    DynArray<TaskID> tasks;
    std::atomic<size_t> done(0);
    std::unique_ptr<std::atomic<int>[]> counters(new std::atomic<int>[workerThreads]);
    Latch latch;

    for (uint32 i = 0; i < workerThreads; ++i)
        counters[i] = 0;

    TaskFunction taskFunc = [&](const TaskContext& context)
    {
        ASSERT_TRUE(context.threadId < workerThreads) << "Unexpected thread id: " << context.threadId;
        counters[context.threadId]++;
        done++;
        latch.Wait();
    };

    // spawn tasks
    for (uint32 i = 0; i < workerThreads; ++i)
    {
        TaskID task = tp.CreateTask(taskFunc);
        ASSERT_NE(NFE_INVALID_TASK_ID, task);
        tasks.PushBack(task);
    }

    // wait until all the tasks are being executed
    while (done < workerThreads)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    latch.Set();

    for (uint32 i = 0; i < workerThreads; ++i)
    {
        tp.WaitForTask(tasks[i]);
    }

    for (uint32 i = 0; i < workerThreads; ++i)
    {
        EXPECT_EQ(1, counters[i]);
    }
}

// Spawn 1000 tasks.
TEST(ThreadPoolSimple, ThousandTasks)
{
    const int numTasks = 1000;
    std::atomic<int> counter(0);
    {
        ThreadPool tp;
        std::vector<TaskID> tasks;
        auto emptyTaskFunc = [&counter](const TaskContext& /* context */) { counter++; };

        for (int i = 0; i < numTasks; ++i)
        {
            TaskID taskID;
            ASSERT_NE(NFE_INVALID_TASK_ID, taskID = tp.CreateTask(emptyTaskFunc));
            tasks.push_back(taskID);
        }

        for (int i = 0; i < numTasks; ++i)
            tp.WaitForTask(tasks[i]);
    }

    EXPECT_EQ(numTasks, counter);
}

// Spawn maximum number of tasks, wait for all and repeat.
TEST(ThreadPoolSimple, WaitForAllTasks)
{
    const int numTasks = 1024;
    ThreadPool tp(numTasks);

    std::atomic<int> counter;
    auto emptyTaskFunc = [&counter](const TaskContext& /* context */) { counter++; };

    for (int j = 0; j < 10; j++)
    {
        counter = 0;
        for (int i = 0; i < numTasks; ++i)
            ASSERT_NE(NFE_INVALID_TASK_ID, tp.CreateTask(emptyTaskFunc));

        tp.WaitForAllTasks();
        EXPECT_EQ(numTasks, counter);
    }
}

// Spawn 1000 tasks.
TEST(ThreadPoolSimple, WaitForThousandTasks)
{
    const int numTasks = 1000;
    std::atomic<int> counter(0);
    {
        ThreadPool tp;
        std::vector<TaskID> tasks;
        auto emptyTaskFunc = [&counter](const TaskContext& /* context */) { counter++; };

        for (int i = 0; i < numTasks; ++i)
        {
            TaskID taskID;
            ASSERT_NE(NFE_INVALID_TASK_ID, taskID = tp.CreateTask(emptyTaskFunc));
            tasks.push_back(taskID);
        }

        tp.WaitForTasks(tasks.data(), tasks.size());
    }

    EXPECT_EQ(numTasks, counter);
}

// create a task dependent on task in progress
TEST(ThreadPoolSimple, DependencyInProgress)
{
    std::atomic<int> counter(0);
    auto funcA = [&counter](const TaskContext& /* context */)
    {
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    auto funcB = [&](const TaskContext& /* context */)
    {
    };

    ThreadPool tp;
    TaskID task0 = 0, task1 = 0;
    ASSERT_NE(NFE_INVALID_TASK_ID, task0 = tp.CreateTask(funcA));
    ASSERT_NE(NFE_INVALID_TASK_ID, task1 = tp.CreateTask(funcB, NFE_INVALID_TASK_ID, task0));
    tp.WaitForTask(task1);
}

// create a task dependent on finished
TEST(ThreadPoolSimple, DependencyFinished)
{
    ThreadPool tp;
    TaskID task0, task1;
    auto func = [](const TaskContext& /* context */) {};

    ASSERT_NE(NFE_INVALID_TASK_ID, task0 = tp.CreateTask(func));
    tp.WaitForTask(task0);

    ASSERT_NE(NFE_INVALID_TASK_ID, task1 = tp.CreateTask(func, NFE_INVALID_TASK_ID, task0));
    tp.WaitForTask(task1);
}

// Create long dependency chain: A -> B -> C -> D ...
// Verify execution order.
TEST(ThreadPoolSimple, DependencyChain)
{
    const int chainLen = 4000; // chain length

    std::atomic<size_t> counters[chainLen];

    auto func = [&](const TaskContext& /* context */, int task)
    {
        if (task == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
        else
        {
            size_t counter = counters[task - 1];
            EXPECT_EQ(1, counter) << "Task #" << task;
        }

        counters[task]++;
    };

    ThreadPool tp;
    for (int i = 0; i < chainLen; ++i)
        counters[i] = 0;

    using namespace std::placeholders;
    TaskID prevTask = 0;
    ASSERT_NE(NFE_INVALID_TASK_ID, prevTask = tp.CreateTask(std::bind(func, _1, 0)));
    for (int i = 1; i < chainLen; ++i)
    {
        ASSERT_NE(NFE_INVALID_TASK_ID, prevTask = tp.CreateTask(std::bind(func, _1, i), NFE_INVALID_TASK_ID, prevTask));
    }
    tp.WaitForTask(prevTask);

    // verify counters
    for (int i = 0; i < chainLen; ++i)
    {
        EXPECT_EQ(1, counters[i]) << "Task #" << i;
    }
}

// Spawn a task inside another recursively
TEST(ThreadPoolSimple, EnqueueInsideTaskRecursive)
{
    const int numTasks = 10000;

    ThreadPool tp;
    std::atomic<int> count(0);
    Latch latch; // last task enqueued latch

    TaskFunction taskFunc = [&](const TaskContext&)
    {
        if (++count < numTasks)
            ASSERT_NE(NFE_INVALID_TASK_ID, tp.CreateTask(taskFunc));
        else
            latch.Set();
    };

    /// spawn first task
    ASSERT_NE(NFE_INVALID_TASK_ID, tp.CreateTask(taskFunc));

    latch.Wait();
    ASSERT_EQ(numTasks, count);
}

// Spawn a child task inside another recursively
TEST(ThreadPoolSimple, EnqueueChildren)
{
    using namespace std::placeholders;
    const int children = 50000;

    ThreadPool tp;
    std::atomic<int> count(0);

    auto childTaskFunc = [&](const TaskContext& /* context */)
    {
        count++;
    };

    auto parentTaskFunc = [&](const TaskContext& context)
    {
        for (int i = 0; i < children; ++i)
        {
            TaskID child = tp.CreateTask(childTaskFunc, context.taskId);
            ASSERT_NE(NFE_INVALID_TASK_ID, child);
        }
    };

    // spawn root task
    TaskID parentTask = tp.CreateTask(parentTaskFunc);
    ASSERT_NE(NFE_INVALID_TASK_ID, parentTask);

    tp.WaitForTask(parentTask);
    EXPECT_EQ(children, count.load());
}

// Spawn 2 child tasks inside another recursively (binary-tree-like structure is created)
TEST(ThreadPoolSimple, EnqueueChildRecursive)
{
    using namespace std::placeholders;
    const int maxDepth = 14;
    const int children = 2;

    ThreadPool tp;
    std::atomic<int> count(0);

    std::function<void(const TaskContext&, int)> taskFunc =
    [&](const TaskContext& context, int depth)
    {
        count++;
        if (depth >= maxDepth)
            return;

        for (int i = 0; i < children; ++i)
        {
            TaskID child = tp.CreateTask(std::bind(taskFunc, _1, depth + 1), context.taskId);
            ASSERT_NE(NFE_INVALID_TASK_ID, child);
        }
    };

    // spawn root task
    TaskID rootTask = tp.CreateTask(std::bind(taskFunc, _1, 0));
    ASSERT_NE(NFE_INVALID_TASK_ID, rootTask);

    tp.WaitForTask(rootTask);
    EXPECT_EQ((1 << (maxDepth + 1)) - 1, count.load());
}
