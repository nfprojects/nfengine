/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Reader/writer lock (mutex) declaration.
 */

#pragma once

#include "../nfCommon.hpp"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(WIN32)


namespace NFE {
namespace Common {

/**
 * Reader/writer lock (mutex).
 * There can be multiple shared locks or only one exclusive lock acquired at the same time.
 */
class RWLock final
{
public:
    NFE_INLINE RWLock();
    NFE_INLINE ~RWLock();

    /**
     * Acquire a lock in exclusive mode.
     */
    NFE_INLINE void AcquireExclusive();

    /**
     * Release a lock that was acquired in exclusive mode.
     */
    NFE_INLINE void ReleaseExclusive();

    /**
     * Acquire a lock in shared mode.
     */
    NFE_INLINE void AcquireShared();

    /**
     * Release a lock that was acquired in shared mode.
     */
    NFE_INLINE void ReleaseShared();

private:
    // RWLock cannot be copied nor moved
    RWLock(const RWLock&) = delete;
    RWLock(RWLock&&) = delete;
    RWLock& operator = (const RWLock&) = delete;
    RWLock& operator = (RWLock&&) = delete;

#if defined(WIN32)
    ::SRWLOCK mLockObject;
#elif defined(__LINUX__) | defined(__linux__)
    ::pthread_rwlock_t* mLockObject;
#endif
};


} // namespace Common
} // namespace NFE


#if defined(WIN32)
#include "Win/LockImpl.hpp"
#elif defined(__LINUX__) | defined(__linux__)
#include "Linux/LockImpl.hpp"
#endif
