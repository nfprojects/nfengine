/**
 * @file
 * @brief  RWSpinLock declaration.
 */

#pragma once

#include "../nfCommon.hpp"

#include <atomic>

namespace NFE {
namespace Common {

/**
 * Reader/writer spin lock.
 * There can be multiple shared locks or only one exclusive lock acquired at the same time.
 *
 * @remarks
 * RWSpinLock should be used ONLY when locking occurs very rarely and for very short period of time.
 * Otherwise, use RWLock.
 */
class RWSpinLock final
{
public:
    NFE_INLINE RWSpinLock() = default;

    /**
     * Try acquiring a lock in exclusive mode.
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
    
    /**
     * Try acquiring a lock in shared mode.
     * @return  True if lock was successfully acquired.
     */
    NFE_INLINE bool TryAcquireShared();

    /**
     * Acquire a lock in shared mode.
     */
    NFE_INLINE void AcquireShared();

    /**
     * Release a lock that was acquired in shared mode.
     */
    NFE_INLINE void ReleaseShared();

private:
    static constexpr int32 UnlockValue = 0;
    static constexpr int32 WriteLockValue = -1;

    RWSpinLock(const RWSpinLock&) = delete;
    RWSpinLock(RWSpinLock&&) = delete;
    RWSpinLock& operator = (const RWSpinLock&) = delete;
    RWSpinLock& operator = (RWSpinLock&&) = delete;

    std::atomic<int32> mValue = UnlockValue;
};


} // namespace Common
} // namespace NFE


#include "RWSpinLockImpl.hpp"
