/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Scoped lock helper class definitions.
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {


/**
 * Helper traits class for acquiring/releasing an generic exclusive lock.
 */
template<typename LockType>
class ExclusiveLockPolicy
{
public:
    NFE_INLINE static void Acquire(LockType& lock) { lock.AcquireExclusive(); }
    NFE_INLINE static void Release(LockType& lock) { lock.ReleaseExclusive(); }
};


/**
 * Helper traits class for acquiring/releasing a generic shared lock.
 */
template<typename LockType>
class SharedLockPolicy
{
public:
    NFE_INLINE static void Acquire(LockType& lock) { lock.AcquireShared(); }
    NFE_INLINE static void Release(LockType& lock) { lock.ReleaseShared(); }
};


/**
 * Scoped lock - helper class for wrapping a generic lock object (e.g. RWLock),
 * that acquires a lock in the constructor and released the lock in the destructor.
 * Locking policy (shared / exclusive) is given in the template argument.
 */
template <typename LockType, typename LockingPolicy>
class ScopedLock final
{
public:
    NFE_INLINE ScopedLock(LockType& lock, bool createLocked = true)
        : mLock(lock)
        , mIsLocked(createLocked)
    {
        if (createLocked)
        {
            LockingPolicy::Acquire(mLock);
        }
    }

    NFE_INLINE ~ScopedLock()
    {
        if (mIsLocked)
        {
            LockingPolicy::Release(mLock);
        }
    }

    NFE_INLINE void Lock()
    {
        if (!mIsLocked)
        {
            LockingPolicy::Acquire(mLock);
            mIsLocked = true;
        }
    }

    NFE_INLINE void Unlock()
    {
        if (mIsLocked)
        {
            LockingPolicy::Release(mLock);
            mIsLocked = false;
        }
    }

    NFE_INLINE constexpr bool IsLocked() const
    {
        return mIsLocked;
    }

    NFE_INLINE LockType& GetLockObject() const
    {
        return mLock;
    }

    ScopedLock& operator = (const ScopedLock&) = default;
    ScopedLock& operator = (ScopedLock&&) = default;

private:
    // ScopedLock cannot be copied
    ScopedLock(const ScopedLock&) = delete;
    ScopedLock(ScopedLock&&) = delete;

    LockType& mLock;
    bool mIsLocked;
};


template <typename LockType>
using ScopedSharedLock = ScopedLock<LockType, SharedLockPolicy<LockType>>;

template <typename LockType>
using ScopedExclusiveLock = ScopedLock<LockType, ExclusiveLockPolicy<LockType>>;


#define NFE_SCOPED_LOCK(lock) ScopedExclusiveLock<decltype(lock)> NFE_UNIQUE_NAME(__lockObject)(lock)
#define NFE_SCOPED_SHARED_LOCK(lock) ScopedSharedLock<decltype(lock)> NFE_UNIQUE_NAME(__lockObject)(lock)

} // namespace Common
} // namespace NFE
