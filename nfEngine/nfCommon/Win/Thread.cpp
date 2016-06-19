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
#define TLS thread_local static const

// Threads' debug name - global variable with implementation per thread
TLS std::string threadDebugName = 'T' + std::to_string(GetCurrentThreadId());
TLS unsigned int threadId = ::GetCurrentThreadId();

bool Thread::SetThreadPriority(std::thread& thread, ThreadPriority priority)
{
    auto result = ::SetThreadPriority(thread.native_handle(), static_cast<int>(priority));
    return (result!= 0);
}

const unsigned int Thread::GetCurrentThreadId()
{
    return threadId;
}

const std::string& Thread::GetCurrentThreadName()
{
    return threadDebugName;
}

} // namespace Common
} // namespace NFE
