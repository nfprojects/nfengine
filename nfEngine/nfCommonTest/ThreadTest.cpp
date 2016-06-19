/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for Thread class.
 */

#include "PCH.hpp"
#include "nfCommon/Thread.hpp"
#include <algorithm>
#include <array>

using namespace NFE::Common;

// Global variables for the tests
const int testThreadNumber = 100;
const int maxThreadNameLength = 16;

// Simple function to test Thread::SetPriority
void simpleFunc()
{
    size_t size = 10000;
    const char value = 123;

    std::vector<char> vectDup(size, value);

    std::unique(vectDup.begin(), vectDup.end());
}

// Simple function to test differences in thread Id
void idTestFunc(size_t* id)
{
    *id = Thread::GetCurrentThreadId();
}

// Simple function to test if setting thread name went good
void setNameTestFunc(const std::string& threadName, bool shouldSuccess)
{
    ASSERT_EQ(shouldSuccess, Thread::SetCurrentThreadName(threadName.c_str()));
}

TEST(ThreadTest, SetPriority)
{
    // SetPriority is not possible on linux OS
    bool assertionVal = false;
#if defined(WIN32)
    assertionVal = true;
#endif

    auto threadTestLambda = [&assertionVal](ThreadPriority pr) {
        std::thread testThread(simpleFunc);
        ASSERT_EQ(assertionVal, Thread::SetThreadPriority(testThread, pr));
        testThread.join();
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
    std::array<size_t, testThreadNumber> testIds;
    std::array<std::thread, testThreadNumber> testThreads;

    for (int i = 0; i < testThreadNumber; i++)
    {
        testThreads[i] = std::thread(idTestFunc, &testIds[i]);
    }

    // Wait till all threads do their work
    for (int i = 0; i < testThreadNumber; i++)
        testThreads[i].join();

    // Create and resize container for unique values
    std::vector<size_t> idsUniqueCopy;
    idsUniqueCopy.reserve(testThreadNumber);

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
    std::array<std::thread, testThreadNumber> testThreads;
    std::string threadName = "thread No";

    for (int i = 0; i < testThreadNumber; i++)
    {
        std::string name = threadName + std::to_string(i);
        name.resize(maxThreadNameLength);
        testThreads[i] = std::thread(setNameTestFunc, name, true);
    }

    // Assure that all threads succeeded
    for (int i = 0; i < testThreadNumber; i++)
        testThreads[i].join();
}

TEST(ThreadTest, SetThreadNameInvalid)
{
    std::array<std::thread, testThreadNumber> testThreads;
    std::string threadName = "thread";

    for (int i = 0; i < testThreadNumber; i++)
    {
        std::string name = threadName + std::to_string(i);
        while (name.size() <= maxThreadNameLength)
            name += name;
        testThreads[i] = std::thread(setNameTestFunc, name, false);
    }

    // Assure that all threads succeeded
    for (int i = 0; i < testThreadNumber; i++)
        testThreads[i].join();
}
