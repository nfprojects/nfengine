/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Reader/writer lock (mutex) Windows implementation.
 */

#pragma once

#include "../RWLock.hpp"


namespace NFE {
namespace Common {


RWLock::RWLock()
{
    ::InitializeSRWLock(&mLockObject);
}

RWLock::~RWLock()
{
     // SRWLOCK doesn't require destruction
}

bool RWLock::TryAcquireExclusive()
{
    return 0 != ::TryAcquireSRWLockExclusive(&mLockObject);
}

void RWLock::AcquireExclusive()
{
    ::AcquireSRWLockExclusive(&mLockObject);
}

void RWLock::ReleaseExclusive()
{
    ::ReleaseSRWLockExclusive(&mLockObject);
}

bool RWLock::TryAcquireShared()
{
    return 0 != ::TryAcquireSRWLockShared(&mLockObject);
}

void RWLock::AcquireShared()
{
    ::AcquireSRWLockShared(&mLockObject);
}

void RWLock::ReleaseShared()
{
    ::ReleaseSRWLockShared(&mLockObject);
}

} // namespace Common
} // namespace NFE
