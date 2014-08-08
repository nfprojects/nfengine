#include "stdafx.h"
#include "../nfCommon/ThreadPool.h"

void TestCallback(void* pUserData, int, int threadID)
{
    int* pVals = (int*)pUserData;
    pVals[threadID]++;
}

void TestLongCallback(void* pUserData, int, int threadID)
{
    int* pVals = (int*)pUserData;
    pVals[threadID]++;
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
}

void TestRandomLengthCallback(void* pUserData, int, int threadID)
{
    int* pVals = (int*)pUserData;
    pVals[threadID]++;
    std::this_thread::sleep_for(std::chrono::microseconds(rand() % 10000));
}


std::vector<std::set<std::thread::id>> gThreadIdList;
void TestThreadIdCallback(void*, int, int threadID)
{
    gThreadIdList[threadID].insert(std::this_thread::get_id());
}

using namespace NFE::Common;

class nfCommonThreadPoolTest : public testing::Test
{
protected:
    // preapre test case enviroment - initialize the engine
    static void SetUpTestCase()
    {
        int ret;
        EXPECT_NO_THROW(ret = tp.Init());
        EXPECT_EQ(0, ret);

        size_t retThreadsCount = tp.GetThreadsCount();
        EXPECT_LT(0, retThreadsCount);

        EXPECT_NO_THROW(mVars = new int [retThreadsCount]);
    }

    // preapre test case enviroment - release the engine
    static void TearDownTestCase()
    {
        int ret;
        EXPECT_NO_THROW(ret = tp.WaitForAllTasks());
        EXPECT_EQ(0, ret);

        delete[] mVars;
    }

    static int* mVars;
    static ThreadPool tp;
};

int* nfCommonThreadPoolTest::mVars = nullptr;
ThreadPool nfCommonThreadPoolTest::tp;

// basic thread pool test - spawn 10000 task instances that increment a value
TEST_F(nfCommonThreadPoolTest, MultipleInstances)
{
    int ret = 0;
    const size_t taskInstances = 10000;
    for (size_t i = 0; i < tp.GetThreadsCount(); i++) mVars[i] = 0;

    TaskID taskID = 0;
    EXPECT_NO_THROW(taskID = tp.AddTask(TestCallback, mVars, taskInstances));
    EXPECT_NO_THROW(ret = tp.WaitForTask(taskID));
    EXPECT_EQ(0, ret);

    // verify tasks output
    int sum = 0;
    for (size_t i = 0; i < tp.GetThreadsCount(); i++) sum += mVars[i];
    EXPECT_EQ(taskInstances, sum);
}

// spawn 1000 tasks that last random amount of time
TEST_F(nfCommonThreadPoolTest, MultipleTasks)
{
    int ret = 0;
    const UINT taskInstances = 1000;
    for (size_t i = 0; i < tp.GetThreadsCount(); i++) mVars[i] = 0;
    std::vector<TaskID> tasks;

    for (int i = 0; i < taskInstances; i++)
        EXPECT_NO_THROW(tasks.push_back(tp.AddTask(TestRandomLengthCallback, mVars, 1)));

    for (int i = 0; i < taskInstances; i++)
    {
        EXPECT_NO_THROW(ret = tp.WaitForTask(tasks[i]));
        EXPECT_EQ(0, ret);
    }

    // verify tasks output
    int sum = 0;
    for (size_t i = 0; i < tp.GetThreadsCount(); i++) sum += mVars[i];
    EXPECT_EQ(taskInstances, sum);
}

// spawn long tasks
TEST_F(nfCommonThreadPoolTest, LongTask)
{
    int ret = 0;
    const size_t taskInstances = std::thread::hardware_concurrency() * 4;
    for (size_t i = 0; i < tp.GetThreadsCount(); i++) mVars[i] = 0;

    TaskID taskID = 0;
    EXPECT_NO_THROW(taskID = tp.AddTask(TestLongCallback, mVars, taskInstances));
    EXPECT_NO_THROW(ret = tp.WaitForTask(taskID));
    EXPECT_EQ(0, ret);

    // verify tasks output
    int sum = 0;
    for (size_t i = 0; i < tp.GetThreadsCount(); i++) sum += mVars[i];
    EXPECT_EQ(taskInstances, sum);
}

// spawn and wait (10000 times)
TEST_F(nfCommonThreadPoolTest, AddAndWait)
{
    int ret = 0;
    const size_t iterations = 10000;
    for (size_t i = 0; i < tp.GetThreadsCount(); i++) mVars[i] = 0;

    TaskID taskID = 0;
    for (size_t i = 0; i < iterations; i++)
    {
        EXPECT_NO_THROW(taskID = tp.AddTask(TestCallback, mVars, 1));
        EXPECT_NO_THROW(ret = tp.WaitForTask(taskID));
        EXPECT_EQ(0, ret);
    }

    // verify tasks output
    int sum = 0;
    for (size_t i = 0; i < tp.GetThreadsCount(); i++) sum += mVars[i];
    EXPECT_EQ(iterations, sum);
}

/*
    Spawn 1000000 task instances and verify "threadId" parameter.
*/
TEST_F(nfCommonThreadPoolTest, ThreadIdTest)
{
    int ret = 0;
    const size_t taskInstances = 500000;
    const size_t threadsNum = tp.GetThreadsCount();

    gThreadIdList.resize(threadsNum);
    for (size_t i = 0; i < tp.GetThreadsCount(); i++)
    {
        gThreadIdList[i].clear();
    }

    TaskID taskID = 0;
    EXPECT_NO_THROW(taskID = tp.AddTask(TestThreadIdCallback, mVars, taskInstances));
    EXPECT_NO_THROW(ret = tp.WaitForTask(taskID));
    EXPECT_EQ(0, ret);

    // verify tasks output
    for (size_t i = 0; i < tp.GetThreadsCount(); i++)
    {
        EXPECT_EQ(1, gThreadIdList[i].size());
    }
}
