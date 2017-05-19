/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Latch class declaration.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/ConditionVariable.hpp"

#include <atomic>


namespace NFE {
namespace Common {

/*
 * @class Latch
 * A tool allowing for threads synchronization.
 */
class NFCOMMON_API Latch final
{
private:
    Mutex mMutex;
    ConditionVariable mCV;
    bool mSet;

public:
    Latch();

    /**
     * Makes all waiting threads return from @p Wait() method.
     */
    void Set();

    /**
     * Waits unitl another thread calls @p Set() method.
     */
    void Wait();

    /**
     * Waits until another thread calls @p Set() method, or until timeout occurs.
     *
     * @param  timeoutMs timeout in milliseconds, 0 to wait infinitely.
     *
     * @return true on success, false when timeout occurred
     */
     bool Wait(const unsigned int timeoutMs);
};

} // namespace Common
} // namespace NFE
