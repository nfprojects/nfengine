/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class declaration.
 */

#pragma once
#include "../nfCommon.hpp"
#include <thread>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(WIN32)

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

using ThreadID = uint32;

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
     * Get current thread's ID
     */
    static ThreadID GetCurrentThreadId();

    /**
     * Get main thread's ID
     */
    static ThreadID GetMainThreadId();

    /**
     * Returns 'true' if called from the main thread.
     */
    static bool IsMainThread();

    /**
     * Get number of logical processors.
     */
    static uint32 GetNumLogicalCPUs();
};

} // namespace Common
} // namespace NFE
