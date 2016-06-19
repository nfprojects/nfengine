/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class definition.
 */

#include "../PCH.hpp"
#include "../Thread.hpp"
#include <thread>


namespace NFE {
namespace Common {

thread_local const unsigned int threadId = ::GetCurrentThreadId();
thread_local const std::string threadDebugName = 'T' + std::to_string(GetCurrentThreadId());

bool Thread::SetThreadPriority(std::thread& thread, ThreadPriority priority)
{
    auto result = ::SetThreadPriority(thread.native_handle(), static_cast<int>(priority));
    return (result!= 0);
}

} // namespace Common
} // namespace NFE
