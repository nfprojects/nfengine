/**
 * @file
 * @brief  Spin Lock declaration.
 */

#pragma once

#include "../nfCommon.hpp"

#include <atomic>

namespace NFE {
namespace Common {

/**
 * SpinLock
 * @remarks
 */
class SpinLock final
{
public:
    NFE_INLINE SpinLock() = default;

    /**
     * Acquire a lock in exclusive mode.
     * @return  True if lock was successfully acquired.
     */
    NFE_INLINE bool TryAcquireExclusive();

    /**
     * Acquire a lock in exclusive mode.
     */
    NFE_INLINE void AcquireExclusive();

    /**
     * Release a lock that was acquired in exclusive mode.
     */
    NFE_INLINE void ReleaseExclusive();

private:
    // SpinLock cannot be copied nor moved
    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock& operator = (const SpinLock&) = delete;
    SpinLock& operator = (SpinLock&&) = delete;

    std::atomic_flag mLocked = ATOMIC_FLAG_INIT;
};


} // namespace Common
} // namespace NFE


#include "SpinLockImpl.hpp"
