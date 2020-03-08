/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Reader/writer lock (mutex) Linux implementation.
 */

#pragma once

#include "../RWLock.hpp"
#include "../Assertion.hpp"


namespace NFE {
namespace Common {


RWLock::RWLock()
{
    int result = ::pthread_rwlock_init(&mLockObject, nullptr);
    NFE_ASSERT(result == 0, "RW lock initialization failed");
}

RWLock::~RWLock()
{
    int result = ::pthread_rwlock_destroy(&mLockObject);
    NFE_ASSERT(result == 0, "RW lock destruction failed");
}

bool RWLock::TryAcquireExclusive()
{
    return 0 == ::pthread_rwlock_trywrlock(&mLockObject);
}

void RWLock::AcquireExclusive()
{
    int result = ::pthread_rwlock_wrlock(&mLockObject);
    NFE_ASSERT(result == 0, "Acquiring exclusive lock failed");
}

void RWLock::ReleaseExclusive()
{
    int result = ::pthread_rwlock_unlock(&mLockObject);
    NFE_ASSERT(result == 0, "Releasing exclusive lock failed");
}

bool RWLock::TryAcquireShared()
{
    return 0 == ::pthread_rwlock_tryrdlock(&mLockObject);
}

void RWLock::AcquireShared()
{
    int result = ::pthread_rwlock_rdlock(&mLockObject);
    NFE_ASSERT(result == 0, "Acquiring shared lock failed");
}

void RWLock::ReleaseShared()
{
    int result = ::pthread_rwlock_unlock(&mLockObject);
    NFE_ASSERT(result == 0, "Releasing shared lock failed");
}


} // namespace Common
} // namespace NFE
