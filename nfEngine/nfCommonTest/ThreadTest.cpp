/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for Thread class.
 */

#include "PCH.hpp"
#include "../nfCommon/Thread.hpp"
#include <algorithm>

using namespace NFE::Common;

// Global variables for the tests
std::vector<unsigned int> testIds;
std::mutex testLock;
const int testThreadNumber = 100;

void simpleFunc()
{
    size_t size = 10000;
    const char value = 123;

    std::vector<char> vectDup(size, value);

    std::unique(vectDup.begin(), vectDup.end());
};

void idTestFunc()
{
    std::lock_guard<std::mutex> guard(testLock);

    testIds.push_back(Thread::GetCurrentThreadId());
};

void nameTestFunc()
{
    const std::string name = Thread::GetCurrentThreadName();
    const unsigned int id = Thread::GetCurrentThreadId();

    const std::string expectedName = 'T' + std::to_string(id);

    ASSERT_EQ(expectedName, name);
};


TEST(ThreadTest, SetPriority)
{
    auto threadTestLambda = [](ThreadPriority pr) {
        std::thread testThread(simpleFunc);
        ASSERT_TRUE(Thread::SetThreadPriority(testThread, pr));
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
    testIds.clear();
    testIds.shrink_to_fit();

    for (int i = 0; i < testThreadNumber; i++)
    {
        std::thread testThread(idTestFunc);
        testThread.join();
    };

    // Wait till all threads do their work
    while (testIds.size() < testThreadNumber) {};

    // Create and resize container for unique values
    std::vector<unsigned int> idsUniqueCopy;
    idsUniqueCopy.reserve(testThreadNumber);

    // Sort in place and then create a unique-values copy
    std::sort(testIds.begin(), testIds.end());
    std::unique_copy(testIds.cbegin(), testIds.cend(), std::back_inserter(idsUniqueCopy));

    // Check for duplicate values - should be none
    std::vector<unsigned int> difference;
    std::set_difference(testIds.cbegin(), testIds.cend(),
                        idsUniqueCopy.cbegin(), idsUniqueCopy.cend(),
                        std::back_inserter(difference));

    ASSERT_TRUE(difference.empty());
}

TEST(ThreadTest, ThreadName)
{
    for (int i = 0; i < testThreadNumber; i++)
    {
        std::thread testThread(nameTestFunc);
        testThread.join();
    };
}
