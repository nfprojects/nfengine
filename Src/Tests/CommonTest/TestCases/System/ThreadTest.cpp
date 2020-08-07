/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for Thread class.
 */

#include "PCH.hpp"
#include "Engine/Common/System/Thread.hpp"

using namespace NFE;
using namespace NFE::Common;

// Simple function to test Thread::SetPriority
static void SimpleFunc(int32 testValue)
{
    ASSERT_EQ(123, testValue);
}

// Simple function to test differences in thread Id
static void idTestFunc(size_t* id)
{
    *id = Thread::GetCurrentThreadID();
}

// Simple function to test if setting thread name went good
static void SetNameTestFunc(const std::string& threadName)
{
    Thread::SetCurrentThreadName(threadName.c_str());
}

TEST(ThreadTest, Constructor_Empty)
{
    Thread thread;
}

TEST(ThreadTest, Run)
{
    Thread thread;
    thread.Run(SimpleFunc, 123);
}

TEST(ThreadTest, RunAndWait)
{
    std::atomic<bool> finished = false;

    {
        Thread thread;
        thread.Run([&]()
        {
            Thread::SleepCurrentThread(0.1);
            finished = true;
        });
    }

    ASSERT_TRUE(finished.load());
}

TEST(ThreadTest, IsMainThread)
{
    Thread thread;
    thread.Run([&]()
    {
        ASSERT_FALSE(Thread::IsMainThread());
    });

    ASSERT_TRUE(Thread::IsMainThread());
}

TEST(ThreadTest, SetPriority)
{
    // SetPriority is not possible on linux OS
    bool assertionVal = false;
#if defined(WIN32)
    assertionVal = true;
#endif

    auto threadTestLambda = [&assertionVal](ThreadPriority pr)
    {
        Thread testThread;
        ASSERT_TRUE(testThread.Run(SimpleFunc, 123));
        ASSERT_EQ(assertionVal, testThread.SetPriority(pr));
        testThread.Wait();
    };

    threadTestLambda(ThreadPriority::Idle);
    threadTestLambda(ThreadPriority::Lowest);
    threadTestLambda(ThreadPriority::BelowNormal);
    threadTestLambda(ThreadPriority::Normal);
    threadTestLambda(ThreadPriority::AboveNormal);
    threadTestLambda(ThreadPriority::Highest);
    threadTestLambda(ThreadPriority::RealTime);
}

TEST(ThreadTest, ThreadId)
{
    const uint32 TestThreadNumber = 10;

    std::array<size_t, TestThreadNumber> testIds;
    std::array<Thread, TestThreadNumber> testThreads;

    for (uint32 i = 0; i < TestThreadNumber; i++)
    {
        testThreads[i].Run(idTestFunc, &testIds[i]);
    }

    // Wait till all threads do their work
    for (uint32 i = 0; i < TestThreadNumber; i++)
    {
        testThreads[i].Wait();
    }

    // Create and resize container for unique values
    std::vector<size_t> idsUniqueCopy;
    idsUniqueCopy.reserve(TestThreadNumber);

    // Sort in place and then create a unique-values copy
    std::sort(testIds.begin(), testIds.end());
    std::unique_copy(testIds.cbegin(), testIds.cend(), std::back_inserter(idsUniqueCopy));

    // Check for duplicate values - should be none
    std::vector<size_t> difference;
    std::set_difference(testIds.cbegin(), testIds.cend(),
                        idsUniqueCopy.cbegin(), idsUniqueCopy.cend(),
                        std::back_inserter(difference));

    ASSERT_TRUE(difference.empty());
}

TEST(ThreadTest, SetThreadNameNormal)
{
    const uint32 TestThreadNumber = 10;

    std::array<Thread, TestThreadNumber> testThreads;
    std::string threadName = "thread No";

    for (uint32 i = 0; i < TestThreadNumber; i++)
    {
        std::string name = threadName + std::to_string(i);
        testThreads[i].Run(SetNameTestFunc, name);
    }

    // Assure that all threads succeeded
    for (uint32 i = 0; i < TestThreadNumber; i++)
    {
        testThreads[i].Wait();
    }
}
