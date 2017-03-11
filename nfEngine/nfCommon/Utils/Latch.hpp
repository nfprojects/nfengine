/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Latch class declaration.
 */

#pragma once

#include "../nfCommon.hpp"

#include <mutex>
#include <condition_variable>
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
    typedef std::unique_lock<std::mutex> Lock;

    std::mutex mMutex;
    std::condition_variable mCV;
    std::atomic<bool> mSet;

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
     * @return true on success, false when timeout occured
     */
     bool Wait(const unsigned int timeoutMs);
};

} // namespace Common
} // namespace NFE
