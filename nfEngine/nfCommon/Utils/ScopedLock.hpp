/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Scoped lock helper class definitions.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/Lock.hpp"


namespace NFE {
namespace Common {


/**
 * Helper traits class for acquiring/releasing an generic exclusive lock.
 */
template<typename LockType>
class ExclusiveLockPolicy
{
public:
    static void Acquire(LockType& lock) { lock.AcquireExclusive(); }
    static void Release(LockType& lock) { lock.ReleaseExclusive(); }
};


/**
 * Helper traits class for acquiring/releasing a generic shared lock.
 */
template<typename LockType>
class SharedLockPolicy
{
public:
    static void Acquire(LockType& lock) { lock.AcquireShared(); }
    static void Release(LockType& lock) { lock.ReleaseShared(); }
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
    ScopedLock(LockType& lock, bool createLocked = true)
        : mLock(lock)
        , mIsLocked(createLocked)
    {
        if (createLocked)
        {
            LockingPolicy::Acquire(mLock);
        }
    }

    ~ScopedLock()
    {
        if (mIsLocked)
        {
            LockingPolicy::Release(mLock);
        }
    }

    void Lock()
    {
        if (!mIsLocked)
        {
            LockingPolicy::Acquire(mLock);
            mIsLocked = true;
        }
    }

    void Unlock()
    {
        if (mIsLocked)
        {
            LockingPolicy::Release(mLock);
            mIsLocked = false;
        }
    }

    constexpr bool IsLocked() const
    {
        return mIsLocked;
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


using ScopedSharedLock = ScopedLock<RWLock, SharedLockPolicy<RWLock>>;
using ScopedExclusiveLock = ScopedLock<RWLock, ExclusiveLockPolicy<RWLock>>;


} // namespace Common
} // namespace NFE
