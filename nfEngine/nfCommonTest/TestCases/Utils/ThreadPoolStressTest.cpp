/**
* @brief Stress tests for ThreadPool class.
*/

#include "PCH.hpp"
#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Utils/Latch.hpp"


using namespace NFE;
using namespace NFE::Common;

/*

TEST(ThreadPoolStress, LotsOfTasks)
{
    const int numTasks = 50000;
    std::atomic<int> counter(0);
    std::vector<std::atomic<bool>> status(numTasks);
    Latch latch;

    {
        ThreadPool tp;
        std::vector<TaskID> tasks;
        auto emptyTaskFunc = [&] (int id)
        {
            status[id] = true;
            counter++;

            if (counter < 8)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (counter == numTasks)
                latch.Set();
        };

        for (int i = 0; i < numTasks; ++i)
        {
            status[i] = false;

            TaskID taskID;
            do
            {
                TaskDesc desc;
                desc.function = [&emptyTaskFunc, i](const TaskContext&) { emptyTaskFunc(i); };
                taskID = tp.CreateAndDispatchTask(desc);
            } while (taskID == NFE_INVALID_TASK_ID);

            tasks.push_back(taskID);
        }

        latch.Wait();
    }

    for (int i = 0; i < numTasks; ++i)
        EXPECT_TRUE(status[i]) << "i=" << i;

    EXPECT_EQ(numTasks, counter);
}

// Spawn lots of tasks with single dependencies.
TEST(ThreadPoolStress, SingleDependency)
{
    const size_t numTasks = 1 << 16;

    struct TaskInfo
    {
        TaskID dependency;
        std::atomic<bool> done;

        TaskInfo() : dependency(NFE_INVALID_TASK_ID), done(false) { }

        TaskInfo(const TaskInfo& other)
        {
            dependency = other.dependency;
            done = other.done.load();
        }
    };

    ThreadPool::Setup setup;
    setup.numInitialTasks = (1 << 17) + 1;

    ThreadPool tp(setup);
    std::vector<TaskID> taskIds;
    std::vector<TaskInfo> tasks;
    tasks.reserve(numTasks);

    auto func = [&](const TaskContext&, size_t id)
    {
        TaskInfo& taskInfo = tasks[id];

        if (taskInfo.dependency != NFE_INVALID_TASK_ID)
        {
            ASSERT_TRUE(tasks[taskInfo.dependency].done) << "Unresolved dependencies";
        }

        // delay first tasks a little bit
        if (id < 50)
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 32));

        taskInfo.done = true;
    };

    for (size_t i = 0; i < numTasks; ++i)
    {
        using namespace std::placeholders;
        TaskInfo task;
        TaskID taskId;

        // generate random dependencies
        size_t range = std::min<size_t>(tasks.size() / 2, 0);
        if (range > 0)
            task.dependency = rand() % range;
        else
            task.dependency = NFE_INVALID_TASK_ID;

        tasks.push_back(task);

        TaskDesc desc;
        desc.function = [&func, i](const TaskContext& context) { func(context, i); };
        desc.dependency = task.dependency;
        desc.waitable = true;
        taskId = tp.CreateAndEnqueueTask(desc);
        ASSERT_EQ(i, taskId);
        taskIds.push_back(taskId);
    }

    // TODO
    // tp.WaitForTasks(taskIds.data(), taskIds.size());
}

*/