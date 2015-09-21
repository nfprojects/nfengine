/**
* @brief Stress tests for ThreadPool class.
*/

#include "PCH.hpp"
#include "../nfCommon/ThreadPool.hpp"
#include "Common.hpp"


using namespace NFE::Common;

// Spawn lots of tasks with complex dependencies.
/*
TEST(ThreadPoolStress, ComplexDependency)
{
    const size_t numTasks = 100000;

    struct TaskInfo
    {
        std::vector<TaskID> deps;
        int done;
        int required;
        size_t instanceNum;
    };

    ThreadPool tp;
    std::vector<TaskID> taskIds;
    std::vector<TaskInfo> tasks;
    TaskID finishTask;
    tasks.reserve(numTasks);

    auto func = [&](size_t instanceId, size_t, size_t id)
    {
        TaskInfo& taskInfo = tasks[id];
        ASSERT_TRUE(instanceId < taskInfo.instanceNum) << "Task instance ID out of bound";

        int depsResolved = 0;
        for (TaskID dependency : taskInfo.deps)
            if (tasks[(size_t)dependency].done)
                depsResolved++;

        ASSERT_GE(taskInfo.required, depsResolved) << "Unresolved dependencies";

        // delay first tasks a little bit
        if (id < 50)
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 32));

        taskInfo.done = 1;
    };

    for (size_t i = 0; i < numTasks; ++i)
    {
        using namespace std::placeholders;
        TaskInfo task;
        TaskID taskId;

        // generate random dependencies
        size_t range = std::min<size_t>(tasks.size(), 0);
        task.required = 0;
        if (range > 0)
        {
            size_t depsNum = rand() % range;
            task.required = rand() % depsNum;
            random_unique(tasks.begin(), tasks.end(), depsNum);
            for (size_t j = 0; j < depsNum; ++j)
                task.deps.push_back(taskIds[i]);
        }

        task.done = 0;
        task.instanceNum = rand() % 8 + 1;
        tasks.push_back(task);
        ASSERT_NO_THROW(taskId = tp.Enqueue(std::bind(func, _1, _2, i),
                                            task.instanceNum, task.deps, task.required));
        ASSERT_EQ(i, taskId);
        taskIds.push_back(taskId);
    }

    ASSERT_NO_THROW(finishTask = tp.Enqueue(TaskFunction(), 1, taskIds));
    tp.WaitForTask(finishTask);
}
*/