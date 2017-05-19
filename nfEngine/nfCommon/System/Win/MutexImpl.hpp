/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mutex Windows implementation.
 */

#pragma once

#include "../Mutex.hpp"


namespace NFE {
namespace Common {


Mutex::Mutex()
{
    ::InitializeSRWLock(&mLockObject);
}

Mutex::~Mutex()
{
    // SRWLOCK doesn't require destruction
}

void Mutex::AcquireExclusive()
{
    ::AcquireSRWLockExclusive(&mLockObject);
}

void Mutex::ReleaseExclusive()
{
    ::ReleaseSRWLockExclusive(&mLockObject);
}


} // namespace Common
} // namespace NFE
