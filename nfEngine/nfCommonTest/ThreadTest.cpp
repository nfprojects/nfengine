/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for Thread class.
 */

#include "PCH.hpp"
#include "../nfCommon/Thread.hpp"

using namespace NFE::Common;

void nameTestFunc()
{
    std::string name = Thread::GetCurrentThreadName();
    unsigned int id = Thread::GetCurrentThreadId();
    std::string expectedName = 'T' + std::to_string(id);

    ASSERT_EQ(expectedName, name);
};

TEST(ThreadTest, SetPriority)
{
    auto threadTestLambda = [](ThreadPriority pr) -> void {
        std::thread testThread(nameTestFunc);
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
