/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class declaration.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

enum class threadPriority;

/**
 * Helper methods for thread interaction
 */
class NFCOMMON_API Thread
{
public:
    static bool SetThreadPriority(std::thread& thread, threadPriority priority);
    static unsigned int GetCurrentThreadID();
};

namespace ThreadDebug {
// Threads' debug name - global variable with implementation per thread
thread_local const std::string threadDebugName = 'T'
                                        + std::to_string(NFE::Common::Thread::GetCurrentThreadID());
}

} // namespace Common
} // namespace NFE
