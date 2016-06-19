/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class declaration.
 */

#pragma once
#include "nfCommon.hpp"
#include <thread>

namespace NFE {
namespace Common {

#if defined(__LINUX__) | defined(__linux__)
enum class ThreadPriority
{
    Idle,
    Lowest,
    BelowNormal,
    Normal,
    AboveNormal,
    Highest,
    RealTime,
};
#elif defined(WIN32)
enum class ThreadPriority
{
    Idle = THREAD_PRIORITY_IDLE,
    Lowest = THREAD_PRIORITY_LOWEST,
    BelowNormal = THREAD_PRIORITY_BELOW_NORMAL,
    Normal = THREAD_PRIORITY_NORMAL,
    AboveNormal = THREAD_PRIORITY_ABOVE_NORMAL,
    Highest = THREAD_PRIORITY_HIGHEST,
    RealTime = THREAD_PRIORITY_TIME_CRITICAL,
};
#endif

#define MAX_THREAD_NAME_LENGTH 16

/**
 * Helper methods for thread interaction
 */
class NFCOMMON_API Thread final
{
public:
    /**
     * Set given threads' priority
     */
    static bool SetThreadPriority(std::thread& thread, ThreadPriority priority);

    /**
     * Set current threads' name
     *
     * @remarks Name can be MAX_THREAD_NAME_LENGTH characters long at max (including '\0')
     */
    static bool SetCurrentThreadName(const char* name);

    /**
     * Get current threads' ID
     */
    static size_t GetCurrentThreadId();
};

} // namespace Common
} // namespace NFE
