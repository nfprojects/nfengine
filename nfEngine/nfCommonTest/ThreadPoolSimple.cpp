/**
* @brief Simple unit tests for ThreadPool class.
*/

#include "PCH.hpp"
#include "../nfCommon/ThreadPool.hpp"
#include "../nfCommon/Latch.hpp"
#include "Common.hpp"


using namespace NFE::Common;

TEST(ThreadPoolSimple, ConstructorDestructor)
{
    ASSERT_NO_THROW(ThreadPool tp);
}

// Spawn an empty task, no waiting.
TEST(ThreadPoolSimple, EmptyTask)
{
    ThreadPool tp;
    auto emptyTaskFunc = [](size_t, size_t) {};
    ASSERT_NO_THROW(tp.CreateTask(emptyTaskFunc, 1));
}

// Destroy a pool while executing a task
TEST(ThreadPoolSimple, DestroyWhileExecuting)
{
    std::unique_ptr<ThreadPool> tp(new ThreadPool);
    auto taskFunc = [](size_t, size_t)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    };

    TaskID task = 0;
    ASSERT_NE(NFE_INVALID_TASK_ID, task = tp->CreateTask(taskFunc, 20));
    ASSERT_NE(NFE_INVALID_TASK_ID, tp->CreateTask(taskFunc, 20, NFE_INVALID_TASK_ID, task));
    ASSERT_NO_THROW(tp.reset());
}

// Spawn small and long task, wait in various orders.
TEST(ThreadPoolSimple, Wait)
{
    ThreadPool tp;
    TaskFunction smallTaskFunc = [](size_t, size_t)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    };
    TaskFunction longTaskFunc = [](size_t, size_t)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    };

    TaskID taskA, taskB, taskC, taskD;

    // wait for already executed
    ASSERT_NE(NFE_INVALID_TASK_ID, taskA = tp.CreateTask(smallTaskFunc, 1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    tp.WaitForTask(taskA);

    // wait for being executed
    ASSERT_NE(NFE_INVALID_TASK_ID, taskB = tp.CreateTask(longTaskFunc, 10));
    tp.WaitForTask(taskB);

    // wait for queued
    ASSERT_NE(NFE_INVALID_TASK_ID, taskC = tp.CreateTask(longTaskFunc, 10));
    ASSERT_NE(NFE_INVALID_TASK_ID, taskD = tp.CreateTask(longTaskFunc, 10));
    tp.WaitForTask(taskD);
    tp.WaitForTask(taskC);
}

// Spawn a task inside another
TEST(ThreadPoolSimple, EnqueueInsideTask)
{
    ThreadPool tp;
    TaskID taskA, taskB;
    std::atomic<int> condition(0);
    Latch latch;

    TaskFunction taskFuncB = [&](size_t instanceId, size_t)
    {
        EXPECT_EQ(1, condition);
        EXPECT_EQ(0, instanceId);
        latch.Set();
    };

    TaskFunction taskFuncA = [&](size_t instanceId, size_t)
    {
        EXPECT_EQ(0, instanceId);
        condition = 1;
        ASSERT_NO_THROW(taskB = tp.CreateTask(taskFuncB, 1, { taskA }));
    };

    ASSERT_NO_THROW(taskA = tp.CreateTask(taskFuncA, 1));

    latch.Wait();
    tp.WaitForTask(taskB);
}

// Verify thread id passed in task callback parameters
TEST(ThreadPoolSimple, ThreadId)
{
    ThreadPool tp;
    TaskID task;
    size_t workerThreads = tp.GetThreadsNumber();
    std::atomic<size_t> done(0);
    std::unique_ptr<std::atomic<int>[]> counters(new std::atomic<int>[workerThreads]);
    Latch latch;

    for (size_t i = 0; i < workerThreads; ++i)
        counters[i] = 0;

    TaskFunction taskFunc = [&](size_t, size_t threadId)
    {
        ASSERT_TRUE(threadId < workerThreads) << "Unexpected thread id: " << threadId;
        counters[threadId]++;
        done++;
        latch.Wait();
    };

    ASSERT_NE(NFE_INVALID_TASK_ID, task = tp.CreateTask(taskFunc, workerThreads));
    while (done < workerThreads)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    latch.Set();
    tp.WaitForTask(task);

    for (size_t i = 0; i < workerThreads; ++i)
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
        auto emptyTaskFunc = [&counter](size_t, size_t) { counter++; };

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

// Spawn 1000 tasks.
TEST(ThreadPoolSimple, ThousandTasksWaitForTasks)
{
    const int numTasks = 1000;
    std::atomic<int> counter(0);
    {
        ThreadPool tp;
        std::vector<TaskID> tasks;
        auto emptyTaskFunc = [&counter](size_t, size_t) { counter++; };

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

TEST(ThreadPoolSimple, InstancesSimple)
{
    const size_t instancesNum = 10000;
    std::unique_ptr<std::atomic<size_t>[]> counters(new std::atomic<size_t>[instancesNum]);

    auto func = [&counters, instancesNum](size_t instanceId, size_t)
    {
        ASSERT_TRUE(instanceId < instancesNum);
        counters[instanceId]++;
    };

    for (size_t i = 0; i < instancesNum; ++i)
        counters[i] = 0;

    ThreadPool tp;
    TaskID task;
    ASSERT_NE(NFE_INVALID_TASK_ID, task = tp.CreateTask(func, instancesNum));
    tp.WaitForTask(task);

    for (size_t i = 0; i < instancesNum; ++i)
        EXPECT_EQ(1, counters[i]) << "Task instance #" << i;
}

// Spawn tasks with various instances numbers.
TEST(ThreadPoolSimple, Instances)
{
    const int tasksNum = 8;
    size_t instancesPerTask[] = { 1, 5, 10, 50, 100, 500, 1000, 5000 };
    // set up counter and test function
    std::atomic<int> counters[tasksNum];
    auto func = [&counters](size_t, size_t, int task)
    {
        counters[task]++;
    };

    ThreadPool tp;
    std::vector<TaskID> tasks;

    for (int i = 0; i < tasksNum; ++i)
        counters[i] = 0;

    // spawn tasks
    for (int i = 0; i < tasksNum; ++i)
    {
        using namespace std::placeholders;
        TaskID task;
        ASSERT_NE(NFE_INVALID_TASK_ID, task = tp.CreateTask(std::bind(func, _1, _2, i),
                                                            instancesPerTask[i]));
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
    auto funcA = [&counter](size_t, size_t)
    {
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    auto funcB = [&](size_t, size_t)
    {
        EXPECT_EQ(numInstances, counter);
    };

    ThreadPool tp;
    TaskID task0 = 0, task1 = 0;
    ASSERT_NE(NFE_INVALID_TASK_ID, task0 = tp.CreateTask(funcA, numInstances));
    ASSERT_NE(NFE_INVALID_TASK_ID, task1 = tp.CreateTask(funcB, numInstances, NFE_INVALID_TASK_ID,
                                                         task0));
    tp.WaitForTask(task1);
}

// create a task dependent on finished
TEST(ThreadPoolSimple, DependencyFinished)
{
    ThreadPool tp;
    TaskID task0, task1;
    auto func = [](size_t, size_t) {};

    ASSERT_NE(NFE_INVALID_TASK_ID, task0 = tp.CreateTask(func, 1));
    tp.WaitForTask(task0);

    ASSERT_NE(NFE_INVALID_TASK_ID, task1 = tp.CreateTask(func, 1, NFE_INVALID_TASK_ID, task0));
    tp.WaitForTask(task1);
}

/*
* Spawn task A (small) and B (big).
* Then spawn 1000 "C" tasks, which are dependent on A and B, but only
* one is required to launch.
*/
/*
TEST(ThreadPoolSimple, DependencyRequiredNum)
{
    int cTaskNum = 10000;

    ThreadPool tp;
    TaskID taskA = 0, taskB = 0, waitTask;
    Latch cFinishLatch;
    std::atomic<int> counterA(0);
    std::atomic<int> counterB(0);

    auto funcA = [&counterA](size_t, size_t)
    {
        counterA++;
    };

    auto funcB = [&counterB, &cFinishLatch](size_t, size_t)
    {
        cFinishLatch.Wait();
        counterB++;
    };

    auto funcC = [&counterA, &counterB, &cFinishLatch](size_t, size_t)
    {
        EXPECT_EQ(1, counterA);
        EXPECT_EQ(0, counterB);
    };

    ASSERT_NE(NFE_INVALID_TASK_ID, taskA = tp.CreateTask(funcA, 1));
    ASSERT_NE(NFE_INVALID_TASK_ID, taskB = tp.CreateTask(funcB, 1));

    std::vector<TaskID> cTasks;
    for (int i = 0; i < cTaskNum; ++i)
    {
        ASSERT_NO_THROW(cTasks.push_back(tp.CreateTask(funcC, 1, { taskA, taskB }, 1)));
    }

    // wait for all "C" tasks
    ASSERT_NE(NFE_INVALID_TASK_ID, waitTask = tp.CreateTask(TaskFunction(), 1, cTasks));
    tp.WaitForTask(waitTask);
    cFinishLatch.Set();
    tp.WaitForTask(taskA);
    tp.WaitForTask(taskB);

    EXPECT_EQ(1, counterA);
    EXPECT_EQ(1, counterB);
}
*/

// Create long dependency chain: A -> B -> C -> D ...
// Verify execution order.
TEST(ThreadPoolSimple, DependencyChain)
{
    const int chainLen = 4000; // chain length
    const int instancesPerTask = 1;

    std::atomic<size_t> counters[chainLen];

    auto func = [&](size_t, size_t, int task)
    {
        if (task == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
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
    ASSERT_NE(NFE_INVALID_TASK_ID, prevTask = tp.CreateTask(std::bind(func, _1, _2, 0),
                                                            instancesPerTask));
    for (int i = 1; i < chainLen; ++i)
    {
        ASSERT_NE(NFE_INVALID_TASK_ID, prevTask = tp.CreateTask(std::bind(func, _1, _2, i),
                                                                instancesPerTask,
                                                                NFE_INVALID_TASK_ID, prevTask));
    }
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

    TaskFunction taskFunc = [&](size_t instanceId, size_t)
    {
        EXPECT_EQ(0, instanceId);

        if (++count < numTasks)
            ASSERT_NE(NFE_INVALID_TASK_ID, tp.CreateTask(taskFunc, 1));
        else
            latch.Set();
    };

    /// spawn first task
    ASSERT_NE(NFE_INVALID_TASK_ID, tp.CreateTask(taskFunc, 1));

    latch.Wait();
    ASSERT_EQ(numTasks, count);
}
