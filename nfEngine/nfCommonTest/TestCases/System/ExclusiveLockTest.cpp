/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Unit tests for exclusive locking objects.
 */

#include "PCH.hpp"
#include "nfCommon/Utils/ScopedLock.hpp"
#include "nfCommon/System/Mutex.hpp"
#include "nfCommon/System/SpinLock.hpp"
#include "nfCommon/System/RWLock.hpp"
#include "nfCommon/System/RWSpinLock.hpp"

using namespace NFE;
using namespace NFE::Common;


template<typename T>
class ExclusiveLockTest : public ::testing::Test
{
};

using LockTestTypes = ::testing::Types<Mutex, RWLock, SpinLock, RWSpinLock>;
TYPED_TEST_CASE(ExclusiveLockTest, LockTestTypes);


TYPED_TEST(ExclusiveLockTest, SimpleLock)
{
    TypeParam lock;
    {
        ScopedExclusiveLock<TypeParam> scopedLock(lock);
        ASSERT_TRUE(scopedLock.IsLocked());
    }
}

TYPED_TEST(ExclusiveLockTest, SimpleTryAcquire)
{
    TypeParam lock;

    ASSERT_TRUE(lock.TryAcquireExclusive());
    ASSERT_FALSE(lock.TryAcquireExclusive());
    lock.ReleaseExclusive();
}

TYPED_TEST(ExclusiveLockTest, SimpleTryAcquire_Locked)
{
    TypeParam lock;

    lock.AcquireExclusive();

    for (uint32 i = 0; i < 1000; ++i)
    {
        ASSERT_FALSE(lock.TryAcquireExclusive());
    }

    lock.ReleaseExclusive();
}

TYPED_TEST(ExclusiveLockTest, Multithreaded)
{
    const uint32 maxIterations = 10000u;

    uint32 counter = 0; // non atomic counter incremented from multiple threads
    TypeParam lock; // for synchronizing counter access

    const auto func = [&]()
    {
        for (uint32 i = 0; i < maxIterations; ++i)
        {
            ScopedExclusiveLock<TypeParam> scopedLock(lock);
            counter++;
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

    ASSERT_EQ(counter, static_cast<uint32>(numThreads) * maxIterations);
}
