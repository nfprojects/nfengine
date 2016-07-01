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

    TaskDesc desc;
    desc.function = [](const TaskContext& /* context */) {};
    ASSERT_NE(NFE_INVALID_TASK_ID, tp.CreateAndEnqueueTask(desc));
}

// Destroy a pool while executing a task
TEST(ThreadPoolSimple, DestroyWhileExecuting)
{
    std::unique_ptr<ThreadPool> tp(new ThreadPool);

    TaskID task = 0;
    TaskDesc desc;
    desc.instancesNum = 20;
    desc.function = [](const TaskContext& /* context */)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    };

    ASSERT_NE(NFE_INVALID_TASK_ID, task = tp->CreateAndEnqueueTask(desc));

    desc.dependency = task;
    ASSERT_NE(NFE_INVALID_TASK_ID, tp->CreateAndEnqueueTask(desc));
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
    TaskDesc desc;

    // wait for already executed
    desc.function = smallTaskFunc;
    desc.waitable = true;
    ASSERT_NE(NFE_INVALID_TASK_ID, taskA = tp.CreateAndEnqueueTask(desc));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    tp.WaitForTask(taskA);

    // wait for being executed
    desc.function = longTaskFunc;
    desc.instancesNum = 10;
    desc.waitable = true;
    ASSERT_NE(NFE_INVALID_TASK_ID, taskB = tp.CreateAndEnqueueTask(desc));
    tp.WaitForTask(taskB);

    // wait for queued
    ASSERT_NE(NFE_INVALID_TASK_ID, taskC = tp.CreateAndEnqueueTask(desc));
    ASSERT_NE(NFE_INVALID_TASK_ID, taskD = tp.CreateAndEnqueueTask(desc));
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

        TaskFunction taskFuncB = [&](const TaskContext& context)
        {
            EXPECT_EQ(1, condition);
            EXPECT_EQ(0, context.instanceId);
            latch.Set();
        };

        TaskFunction taskFuncA = [&](const TaskContext& context)
        {
            EXPECT_EQ(0, context.instanceId);
            condition = 1;

            TaskDesc desc;
            desc.function = taskFuncB;
            desc.parent = context.taskId;
            desc.waitable = true;
            ASSERT_NE(NFE_INVALID_TASK_ID, task = tp.CreateAndEnqueueTask(desc));
        };

        TaskDesc desc;
        desc.function = taskFuncA;
        ASSERT_NE(NFE_INVALID_TASK_ID, tp.CreateAndEnqueueTask(desc));

        latch.Wait();
        tp.WaitForTask(task);
    }
}

// Verify thread id passed in task callback parameters
TEST(ThreadPoolSimple, ThreadId)
{
    ThreadPool tp;
    TaskID task;
    const uint32 workerThreads = tp.GetThreadsNumber();

    TaskFunction taskFunc = [&](const TaskContext& context)
    {
        ASSERT_TRUE(context.threadId < workerThreads) << "Unexpected thread id: " << context.threadId;
    };

    TaskDesc desc;
    desc.function = taskFunc;
    desc.instancesNum = static_cast<uint32>(workerThreads);
    desc.waitable = true;
    ASSERT_NE(NFE_INVALID_TASK_ID, task = tp.CreateAndEnqueueTask(desc));
    tp.WaitForTask(task);
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

            TaskDesc desc;
            desc.function = emptyTaskFunc;
            desc.waitable = true;
            ASSERT_NE(NFE_INVALID_TASK_ID, taskID = tp.CreateAndEnqueueTask(desc));
            tasks.push_back(taskID);
        }

        for (int i = 0; i < numTasks; ++i)
            tp.WaitForTask(tasks[i]);
    }

    EXPECT_EQ(numTasks, counter);
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

            TaskDesc desc;
            desc.function = emptyTaskFunc;
            desc.waitable = true;
            ASSERT_NE(NFE_INVALID_TASK_ID, taskID = tp.CreateAndEnqueueTask(desc));
            tasks.push_back(taskID);
        }

        tp.WaitForTasks(tasks.data(), tasks.size());
    }

    EXPECT_EQ(numTasks, counter);
}

TEST(ThreadPoolSimple, InstancesSimple)
{
    const size_t instancesNum = 10000;
    std::unique_ptr<std::atomic<size_t>[]> counters(new std::atomic<size_t>[instancesNum]);

    auto func = [&counters, instancesNum](const TaskContext& context)
    {
        ASSERT_TRUE(context.instanceId < instancesNum);
        counters[context.instanceId]++;
    };

    for (size_t i = 0; i < instancesNum; ++i)
        counters[i] = 0;

    ThreadPool tp;
    TaskID task;

    TaskDesc desc;
    desc.function = func;
    desc.instancesNum = instancesNum;
    desc.waitable = true;
    ASSERT_NE(NFE_INVALID_TASK_ID, task = tp.CreateAndEnqueueTask(desc));
    tp.WaitForTask(task);

    for (size_t i = 0; i < instancesNum; ++i)
        EXPECT_EQ(1, counters[i]) << "Task instance #" << i;
}

// Spawn tasks with various instances numbers.
TEST(ThreadPoolSimple, Instances)
{
    const int tasksNum = 8;
    uint32 instancesPerTask[] = { 1, 5, 10, 50, 100, 500, 1000, 5000 };
    // set up counter and test function
    std::atomic<int> counters[tasksNum];

    ThreadPool tp;
    std::vector<TaskID> tasks;

    for (int i = 0; i < tasksNum; ++i)
        counters[i] = 0;

    // spawn tasks
    for (int i = 0; i < tasksNum; ++i)
    {
        using namespace std::placeholders;
        TaskID task;

        TaskDesc desc;
        desc.function = [i, &counters](const TaskContext&) { counters[i]++; };
        desc.instancesNum = instancesPerTask[i];
        desc.waitable = true;
        ASSERT_NE(NFE_INVALID_TASK_ID, task = tp.CreateAndEnqueueTask(desc));
        tasks.push_back(task);
    }

    // verify counters
    for (int i = 0; i < tasksNum; ++i)
    {
        tp.WaitForTask(tasks[i]);
        EXPECT_EQ(instancesPerTask[i], counters[i]) << "Task #" << i;
    }
}

// create a task dependent on task in progress
TEST(ThreadPoolSimple, DependencyInProgress)
{
    const int numInstances = 10;

    std::atomic<int> counter(0);
    auto funcA = [&counter](const TaskContext& /* context */)
    {
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    auto funcB = [&](const TaskContext& /* context */)
    {
        EXPECT_EQ(numInstances, counter);
    };

    ThreadPool tp;
    TaskID task0 = 0, task1 = 0;
    TaskDesc desc;

    desc.function = funcA;
    desc.instancesNum = numInstances;
    desc.waitable = true;
    ASSERT_NE(NFE_INVALID_TASK_ID, task0 = tp.CreateAndEnqueueTask(desc));

    desc.dependency = task0;
    ASSERT_NE(NFE_INVALID_TASK_ID, task1 = tp.CreateAndEnqueueTask(desc));
    tp.WaitForTask(task1);
    tp.WaitForTask(task0);
}

// create a task dependent on finished
TEST(ThreadPoolSimple, DependencyFinished)
{
    ThreadPool tp;
    TaskID task0, task1;
    Latch latch;
    TaskDesc desc;

    desc.function = [&](const TaskContext& /* context */) { latch.Set();  };
    desc.waitable = true;
    ASSERT_NE(NFE_INVALID_TASK_ID, task0 = tp.CreateAndEnqueueTask(desc));
    latch.Wait();

    desc.function = [&](const TaskContext& /* context */) {};
    desc.dependency = task0;
    desc.waitable = true;
    ASSERT_NE(NFE_INVALID_TASK_ID, task1 = tp.CreateAndEnqueueTask(desc));

    tp.WaitForTask(task0);
    tp.WaitForTask(task1);
}

// Create long dependency chain: A -> B -> C -> D ...
// Verify execution order.
TEST(ThreadPoolSimple, DependencyChain)
{
    const int chainLen = 4000; // chain length
    const int instancesPerTask = 1;

    Latch latch;
    std::atomic<size_t> counters[chainLen];

    auto func = [&](int task)
    {
        if (task == 0)
            latch.Wait();
        else
        {
            size_t counter = counters[task - 1];
            EXPECT_EQ(instancesPerTask, counter) << "Task #" << task;
        }

        counters[task]++;
    };

    ThreadPool tp;
    for (int i = 0; i < chainLen; ++i)
        counters[i] = 0;

    using namespace std::placeholders;
    TaskID prevTask = 0;

    TaskDesc desc;
    desc.function = [&func](const TaskContext&) { func(0); };
    desc.instancesNum = instancesPerTask;
    ASSERT_NE(NFE_INVALID_TASK_ID, prevTask = tp.CreateAndEnqueueTask(desc));
    for (int i = 1; i < chainLen; ++i)
    {
        desc.function = [&func, i](const TaskContext&) { func(i); };
        desc.dependency = prevTask;
        desc.waitable = true;
        ASSERT_NE(NFE_INVALID_TASK_ID, prevTask = tp.CreateAndEnqueueTask(desc));
    }

    latch.Set();
    tp.WaitForTask(prevTask);

    // verify counters
    for (int i = 0; i < chainLen; ++i)
    {
        EXPECT_EQ(instancesPerTask, counters[i]) << "Task #" << i;
    }
}

// Spawn a task inside another recursively
TEST(ThreadPoolSimple, EnqueueInsideTaskRecursive)
{
    const int numTasks = 10000;

    ThreadPool tp;
    std::atomic<int> count(0);
    Latch latch; // last task enqueued latch

    TaskFunction taskFunc = [&](const TaskContext& context)
    {
        EXPECT_EQ(0, context.instanceId);

        if (++count < numTasks)
        {
            TaskDesc desc;
            desc.function = taskFunc;
            ASSERT_NE(NFE_INVALID_TASK_ID, tp.CreateAndEnqueueTask(desc));
        }
        else
            latch.Set();
    };

    /// spawn first task
    TaskDesc desc;
    desc.function = taskFunc;
    ASSERT_NE(NFE_INVALID_TASK_ID, tp.CreateAndEnqueueTask(desc));

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
            TaskDesc desc;
            desc.function = childTaskFunc;
            desc.parent = context.taskId;
            TaskID child = tp.CreateAndEnqueueTask(desc);
            ASSERT_NE(NFE_INVALID_TASK_ID, child);
        }
    };

    // spawn root task
    TaskDesc desc;
    desc.function = parentTaskFunc;
    desc.waitable = true;
    TaskID parentTask = tp.CreateAndEnqueueTask(desc);
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

    std::function<void(const TaskContext&, int)> func;
    func = [&](const TaskContext& context, int depth)
    {
        count++;
        if (depth >= maxDepth)
            return;

        for (int i = 0; i < children; ++i)
        {
            TaskDesc desc;
            desc.function = [&func, depth](const TaskContext& context) { func(context, depth + 1); };
            desc.parent = context.taskId;

            TaskID child = tp.CreateAndEnqueueTask(desc);
            ASSERT_NE(NFE_INVALID_TASK_ID, child);
        }
    };

    // spawn root task
    TaskDesc desc;
    desc.function = [&func](const TaskContext& context) { func(context, 0); };
    desc.waitable = true;
    TaskID rootTask = tp.CreateAndEnqueueTask(desc);
    ASSERT_NE(NFE_INVALID_TASK_ID, rootTask);

    tp.WaitForTask(rootTask);
    EXPECT_EQ((1 << (maxDepth + 1)) - 1, count.load());
}
