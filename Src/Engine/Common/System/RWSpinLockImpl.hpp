/**
 * @file
 * @brief  Spin Lock implementation.
 */

#pragma once

#include "RWSpinLock.hpp"
#include "Thread.hpp"
#include "Assertion.hpp"

namespace NFE {
namespace Common {


bool RWSpinLock::TryAcquireShared()
{
    int32 expected = mValue.load(std::memory_order_relaxed);
    if (expected != WriteLockValue)
    {
        int32 desired = 1 + expected;
        if (atomic_compare_exchange_weak_explicit(&mValue, &expected, desired, std::memory_order_relaxed, std::memory_order_relaxed))
        {
            atomic_thread_fence(std::memory_order_acquire);
            return true;
        }
    }

    return false;
}

void RWSpinLock::AcquireShared()
{
    for (;;)
    {
        int32 expected = mValue.load(std::memory_order_relaxed);
        if (expected != WriteLockValue)
        {
            int32 desired = 1 + expected;
            if (atomic_compare_exchange_weak_explicit(&mValue, &expected, desired, std::memory_order_relaxed, std::memory_order_relaxed))
            {
                break;
            }
        }

        Thread::YieldCurrentThread();
    }

    atomic_thread_fence(std::memory_order_acquire);
}

void RWSpinLock::ReleaseShared()
{
    atomic_thread_fence(std::memory_order_release);

    const uint32 prevValue = mValue--;
    NFE_ASSERT(prevValue > 0, "Invalid lock usage");
}

bool RWSpinLock::TryAcquireExclusive()
{
    int32 expected = UnlockValue;
    int32 desired = WriteLockValue;
    if (atomic_compare_exchange_weak_explicit(&mValue, &expected, desired, std::memory_order_relaxed, std::memory_order_relaxed))
    {
        atomic_thread_fence(std::memory_order_acquire);
        return true;
    }

    return false;
}

void RWSpinLock::AcquireExclusive()
{
    for (;;)
    {
        int32 expected = UnlockValue;
        int32 desired = WriteLockValue;
        if (atomic_compare_exchange_weak_explicit(&mValue, &expected, desired, std::memory_order_relaxed, std::memory_order_relaxed))
        {
            break;
        }

        Thread::YieldCurrentThread();
    }

    atomic_thread_fence(std::memory_order_acquire);
}

void RWSpinLock::ReleaseExclusive()
{
    atomic_thread_fence(std::memory_order_release);

    mValue = UnlockValue;
}

} // namespace Common
} // namespace NFE
