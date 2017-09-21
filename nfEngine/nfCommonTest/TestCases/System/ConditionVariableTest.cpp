/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Unit tests for ScopedLock and RWLock classes.
 */

#include "PCH.hpp"
#include "nfCommon/Utils/ScopedLock.hpp"
#include "nfCommon/System/ConditionVariable.hpp"


using namespace NFE;
using namespace NFE::Common;

TEST(ConditionVariableTest, WaitFor_Timeout)
{
    Mutex mutex;
    ConditionVariable cv;
    const uint32 timeout = 20;

    {
        ScopedMutexLock lock(mutex);
        EXPECT_FALSE(cv.WaitFor(lock, timeout));
    }
}

TEST(ConditionVariableTest, WaitFor_Multithreaded)
{
    Mutex mutex;
    ConditionVariable cv;
    bool signaled = false;

    const auto threadFunc = [&]()
    {
        ScopedMutexLock lock(mutex);
        signaled = true;
        cv.SignalOne();
    };

    std::thread thread(threadFunc);

    ScopedMutexLock lock(mutex);
    while (!signaled)
    {
        EXPECT_TRUE(cv.WaitFor(lock, 1000));
    }

    thread.join();
}


TEST(ConditionVariableTest, WaitMultipleTimes)
{
    Mutex mutex;
    ConditionVariable cv;
    int counter = 0;
    const int target = 100;

    const auto threadFunc = [&]()
    {
        for (int i = 0; i < target; ++i)
        {
            ScopedMutexLock lock(mutex);
            counter++;
            cv.SignalOne();
        }
    };

    std::thread thread(threadFunc);

    {
        ScopedMutexLock lock(mutex);
        while (counter < target)
        {
            cv.Wait(lock);
        }
    }

    thread.join();
}
