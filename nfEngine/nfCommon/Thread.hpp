/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class declaration.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

#if defined(__LINUX__) | defined(__linux__)
enum class ThreadPriority
{
    Idle = 1,
    Lowest = 16,
    BelowNormal = 33,
    Normal = 50,
    AboveNormal = 66,
    Highest = 83,
    RealTime = 99,
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

// Threads' id (implementation per thread)
extern thread_local const unsigned int threadId;

// Threads' debug name (implementation per thread)
extern thread_local const std::string threadDebugName;

/**
 * Helper methods for thread interaction
 */
class NFCOMMON_API Thread final
{
public:
    static bool SetThreadPriority(std::thread& thread, ThreadPriority priority);

    NFE_INLINE static const unsigned int GetCurrentThreadId()
    {
        return threadId;
    }

    NFE_INLINE static const std::string& GetCurrentThreadName()
    {
        return threadDebugName;
    }
};

} // namespace Common
} // namespace NFE
