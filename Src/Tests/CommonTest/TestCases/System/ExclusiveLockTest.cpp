/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Unit tests for exclusive locking objects.
 */

#include "PCH.hpp"
#include "Engine/Common/Utils/ScopedLock.hpp"
#include "Engine/Common/System/Mutex.hpp"
#include "Engine/Common/System/SpinLock.hpp"
#include "Engine/Common/System/RWLock.hpp"
#include "Engine/Common/System/RWSpinLock.hpp"
#include "Engine/Common/System/Thread.hpp"
#include "Engine/Common/Containers/DynArray.hpp"

using namespace NFE;
using namespace NFE::Common;


template<typename T>
class ExclusiveLockTest : public ::testing::Test
{
};

using LockTestTypes = ::testing::Types<Mutex, RWLock, SpinLock, RWSpinLock>;
TYPED_TEST_SUITE(ExclusiveLockTest, LockTestTypes);


TYPED_TEST(ExclusiveLockTest, Simple_Lock)
{
    TypeParam lock;
    {
        ScopedExclusiveLock<TypeParam> scopedLock(lock);
        ASSERT_TRUE(scopedLock.IsLocked());
    }
}

TYPED_TEST(ExclusiveLockTest, Simple_TwoLocks)
{
    TypeParam lockA, lockB;
    {
        NFE_SCOPED_LOCK(lockA);
        NFE_SCOPED_LOCK(lockB);
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

    const uint32 numThreads = std::min(1024u, Thread::GetSystemThreadsCount());
    DynArray<Thread> threads(numThreads);

    for (uint32 i = 0; i < numThreads; ++i)
    {
        threads[i].Run(func);
    }

    threads.Clear();

    ASSERT_EQ(counter, static_cast<uint32>(numThreads) * maxIterations);
}
