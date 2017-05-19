/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  ConditionVariable declaration.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Utils/ScopedLock.hpp"


namespace NFE {
namespace Common {


class ConditionVariable final
{
public:
    NFE_INLINE ConditionVariable();
    NFE_INLINE ~ConditionVariable();

    /**
     * Wait until the condition variable is signaled.
     * @note    This method must be called withing locked mutex.
     *          Otherwise behavior is undefined.
     */
    NFE_INLINE void Wait(ScopedMutexLock& lock);

    /**
     * Wait until the condition variable is signaled (with timeout).
     * @return  False if timeout was reached, true if signaled.
     */
    NFE_INLINE bool WaitFor(ScopedMutexLock& lock, uint32 milliseconds);

    /**
     * Signal one waiting thread.
     */
    NFE_INLINE void SignalOne();

    /**
     * Signal all waiting threads.
     */
    NFE_INLINE void SignalAll();

private:
    // ConditionVariable cannot be copied nor moved
    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable(ConditionVariable&&) = delete;
    ConditionVariable& operator = (const ConditionVariable&) = delete;
    ConditionVariable& operator = (ConditionVariable&&) = delete;

#if defined(WIN32)
    ::CONDITION_VARIABLE mConditionVariableObject;
#elif defined(__LINUX__) | defined(__linux__)
    ::pthread_cond_t mConditionVariableObject;
#endif
};


} // namespace Common
} // namespace NFE


#if defined(WIN32)
#include "Win/ConditionVariableImpl.hpp"
#elif defined(__LINUX__) | defined(__linux__)
#include "Linux/ConditionVariableImpl.hpp"
#endif
