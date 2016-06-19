/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for Thread class.
 */

#include "PCH.hpp"
#include "../nfCommon/Thread.hpp"
#include <algorithm>
#include <array>

using namespace NFE::Common;

// Global variables for the tests
const int testThreadNumber = 100;

// Simple function to test Thread::SetPriority
void simpleFunc()
{
    size_t size = 10000;
    const char value = 123;

    std::vector<char> vectDup(size, value);

    std::unique(vectDup.begin(), vectDup.end());
};

// Simple function to test differences in thread Id
void idTestFunc(unsigned int* id)
{
    *id = Thread::GetCurrentThreadId();
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
    std::array<unsigned int, testThreadNumber> testIds;
    std::array<std::thread, testThreadNumber> testThreads;

    for (int i = 0; i < testThreadNumber; i++)
        testThreads[i] = std::thread(idTestFunc, &testIds[i]);

    // Wait till all threads do their work
    for (int i = 0; i < testThreadNumber; i++)
        testThreads[i].join();

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