/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Condition Variable Windows implementation.
 */

#pragma once

#include "../ConditionVariable.hpp"
#include "../System/Assertion.hpp"
#include "Mutex.hpp"


namespace NFE {
namespace Common {


ConditionVariable::ConditionVariable()
{
    ::InitializeConditionVariable(&mConditionVariableObject);
}

ConditionVariable::~ConditionVariable()
{
    // CONDITION_VARIABLE doesn't require destruction
}

void ConditionVariable::Wait(ScopedMutexLock& lock)
{
    NFE_ASSERT(lock.IsLocked(), "Scoped lock must be in locked state when waiting on ConditionVariable");

    Mutex& mutex = lock.GetLockObject();
    const DWORD ret = ::SleepConditionVariableSRW(&mConditionVariableObject, mutex.GetLockObject(), INFINITE, 0);
    NFE_ASSERT(ret != 0, "SleepConditionVariableCS failed, last error code: %d", GetLastError());
}

bool ConditionVariable::WaitFor(ScopedMutexLock& lock, uint32 milliseconds)
{
    NFE_ASSERT(lock.IsLocked(), "Scoped lock must be in locked state when waiting on ConditionVariable");

    Mutex& mutex = lock.GetLockObject();
    const DWORD ret = ::SleepConditionVariableSRW(&mConditionVariableObject, mutex.GetLockObject(), milliseconds, 0);
    if (ret == 0)
    {
        // in case of timeout, SleepConditionVariableCS should fail with ERROR_TIMEOUT error code
        const DWORD errorCode = GetLastError();
        NFE_ASSERT(errorCode == ERROR_TIMEOUT, "SleepConditionVariableCS failed, last error code: %d", errorCode);
    }

    return ret != 0;
}

void ConditionVariable::SignalOne()
{
    ::WakeConditionVariable(&mConditionVariableObject);
}

void ConditionVariable::SignalAll()
{
    ::WakeAllConditionVariable(&mConditionVariableObject);
}


} // namespace Common
} // namespace NFE
