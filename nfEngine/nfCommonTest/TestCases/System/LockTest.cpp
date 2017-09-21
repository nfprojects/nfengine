/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Unit tests for ScopedLock and RWLock classes.
 */

#include "PCH.hpp"
#include "nfCommon/Utils/ScopedLock.hpp"
#include "nfCommon/System/Thread.hpp"
#include "nfCommon/Utils/Latch.hpp"


using namespace NFE;
using namespace NFE::Common;


TEST(LockTest, SimpleSharedLock)
{
    RWLock lock;
    {
        ScopedSharedLock scopedLock(lock);
        ASSERT_TRUE(scopedLock.IsLocked());
    }
}

TEST(LockTest, SimpleExclusiveLock)
{
    RWLock lock;
    {
        ScopedExclusiveLock scopedLock(lock);
        ASSERT_TRUE(scopedLock.IsLocked());
    }
}

TEST(LockTest, ExlusiveLock_Multithreaded)
{
    const uint32 maxIterations = 10000;

    uint32 sharedCounter = 0; // non atomic counter incremented from multiple threads
    RWLock lock; // for synchronizing counter access

    const auto func = [&sharedCounter, &lock, maxIterations]()
    {
        for (uint32 i = 0; i < maxIterations; ++i)
        {
            ScopedExclusiveLock scopedLock(lock);
            sharedCounter++;
        }
    };

    const size_t numThreads = std::min<size_t>(1024, std::thread::hardware_concurrency());
    std::vector<std::thread> threads;
    for (size_t i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(func);
    }

    for (size_t i = 0; i < numThreads; ++i)
    {
        threads[i].join();
    }

    ASSERT_EQ(sharedCounter, static_cast<uint32>(numThreads) * maxIterations);
}

TEST(LockTest, SharedLock_Multithreaded)
{
    RWLock testLock; // the lock being tested

    using LockType = ScopedMutexLock;

    uint32 counter;
    ConditionVariable hasEnteredCV;
    Mutex hasEnteredMutex;

    bool canContinue;
    ConditionVariable canContinueCV;
    Mutex canContinueMutex;


    const auto func = [&]()
    {
        ScopedSharedLock scopedLock(testLock);

        // notify main thread about reaching synchronization point
        {
            LockType lock(hasEnteredMutex);
            counter++;
            hasEnteredCV.SignalOne();
        }

        // if we are here, it means that all the threads are inside shared lock

        // exit the shared lock only when notified by the main thread
        {
            LockType lock(canContinueMutex);
            while (!canContinue)
            {
                canContinueCV.Wait(lock);
            }
        }
    };

    counter = 0;
    canContinue = false;

    // spawn threads
    const size_t numThreads = std::min<size_t>(1024, std::thread::hardware_concurrency());
    std::vector<std::thread> threads;
    for (size_t i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(func);
    }

    // wait until all the threads reach synchronization point
    {
        LockType lock(hasEnteredMutex);
        while (counter < static_cast<uint32>(numThreads))
        {
            hasEnteredCV.Wait(lock);
        }
    }

    // when program execution reaches this place, this means
    // that counter has been incremented to maximum value
    ASSERT_EQ(counter, static_cast<uint32>(numThreads));

    // notify threads to continue
    {
        LockType lock(canContinueMutex);
        canContinue = true;
        canContinueCV.SignalAll();
    }

    for (size_t i = 0; i < numThreads; ++i)
    {
        threads[i].join();
    }

    ASSERT_EQ(counter, static_cast<uint32>(numThreads));
}
