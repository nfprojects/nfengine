/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Unit tests for Mutex class.
 */

#include "PCH.hpp"
#include "nfCommon/Utils/ScopedLock.hpp"


using namespace NFE;
using namespace NFE::Common;


namespace {

const uint32 maxIterations = 10000u;

} // namespace


TEST(MutexText, SimpleLock)
{
    Mutex lock;
    {
        ScopedMutexLock scopedLock(lock);
        ASSERT_TRUE(scopedLock.IsLocked());
    }
}

TEST(MutexText, Multithreaded)
{
    uint32 sharedCounter = 0; // non atomic counter incremented from multiple threads
    Mutex lock; // for synchronizing counter access

    const auto func = [&sharedCounter, &lock]()
    {
        for (uint32 i = 0; i < maxIterations; ++i)
        {
            ScopedMutexLock scopedLock(lock);
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
