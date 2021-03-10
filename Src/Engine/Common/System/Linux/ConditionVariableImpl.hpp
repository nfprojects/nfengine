/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Reader/writer lock (mutex) Linux implementation.
 */

#pragma once

#include "../ConditionVariable.hpp"
#include "../Assertion.hpp"

#include <sys/time.h>
#include <cerrno>


namespace NFE {
namespace Common {


ConditionVariable::ConditionVariable()
{
    int result = ::pthread_cond_init(&mConditionVariableObject, nullptr);
    NFE_ASSERT(result == 0, "Condition variable initialization failed, error code = %i", errno);
}

ConditionVariable::~ConditionVariable()
{
    int result = ::pthread_cond_destroy(&mConditionVariableObject);
    NFE_ASSERT(result == 0, "Condition variable destruction failed, error code = %i", errno);
}

void ConditionVariable::Wait(ScopedExclusiveLock<Mutex>& lock)
{
    int result = ::pthread_cond_wait(&mConditionVariableObject, lock.GetLockObject().GetMutexObject());
    NFE_ASSERT(result == 0, "Condition variable waiting failed, error code = %i", errno);
}

bool ConditionVariable::WaitFor(ScopedExclusiveLock<Mutex>& lock, uint32 milliseconds)
{
    ::timeval tv;
    gettimeofday(&tv, NULL);

    // well, I didn't designed pthreads API...
    ::timespec ts;
    ts.tv_sec = time(NULL) + milliseconds / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000000 * (milliseconds % 1000);
    ts.tv_sec += ts.tv_nsec / 1000000000;
    ts.tv_nsec %= 1000000000;

    int ret = ::pthread_cond_timedwait(&mConditionVariableObject, lock.GetLockObject().GetMutexObject(), &ts);
    if (ret == ETIMEDOUT)
    {
        // timeout
        return false;
    }

    NFE_ASSERT(ret == 0, "Condition variable timed waiting failed, error code = %i", errno);
    return true;
}

void ConditionVariable::SignalOne()
{
    int result = ::pthread_cond_signal(&mConditionVariableObject);
    NFE_ASSERT(result == 0, "Condition variable signal failed, error code = %i", errno);
}

void ConditionVariable::SignalAll()
{
    int result = ::pthread_cond_broadcast(&mConditionVariableObject);
    NFE_ASSERT(result == 0, "Condition variable broadcast failed, error code = %i", errno);
}


} // namespace Common
} // namespace NFE
