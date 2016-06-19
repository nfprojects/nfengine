/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for Thread class.
 */

#include "PCH.hpp"
#include "../nfCommon/Thread.hpp"

using namespace NFE::Common;

void nameTestFunc(std::vector<unsigned int>& ids)
{
    std::string name = Thread::GetCurrentThreadName();
    unsigned int id = Thread::GetCurrentThreadId();
    ASSERT_EQ(0, std::count(ids.begin(), ids.end(), id));
    ids.push_back(id);
    std::string expectedName = 'T' + std::to_string(id);

    ASSERT_EQ(expectedName, name);
};

TEST(ThreadTest, SetPriority)
{
    std::vector<unsigned int> ids;
    auto threadTestLambda = [&ids](ThreadPriority pr) -> void {
        std::thread testThread(nameTestFunc, ids);
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
