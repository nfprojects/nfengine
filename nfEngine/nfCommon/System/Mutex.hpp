/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mutex declaration.
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


class Mutex final
{
public:
    NFE_INLINE Mutex();
    NFE_INLINE ~Mutex();

    /**
     * Acquire a lock in exclusive mode.
     */
    NFE_INLINE void AcquireExclusive();

    /**
     * Release a lock that was acquired in exclusive mode.
     */
    NFE_INLINE void ReleaseExclusive();

private:
    friend class ConditionVariable;

    // Mutex cannot be copied nor moved
    Mutex(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;
    Mutex& operator = (const Mutex&) = delete;
    Mutex& operator = (Mutex&&) = delete;

#if defined(WIN32)

    ::SRWLOCK mLockObject;
    ::SRWLOCK* GetLockObject() { return &mLockObject; }

#elif defined(__LINUX__) | defined(__linux__)

    ::pthread_mutex_t mMutexObject;
    ::pthread_mutex_t* GetMutexObject() { return &mMutexObject; }

#endif // defined(WIN32)
};


} // namespace Common
} // namespace NFE


#if defined(WIN32)
#include "Win/MutexImpl.hpp"
#elif defined(__LINUX__) | defined(__linux__)
#include "Linux/MutexImpl.hpp"
#endif
