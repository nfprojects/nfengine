/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class definition.
 */

#include "PCH.hpp"
#include "../Thread.hpp"
#include <thread>
#include <pthread.h>


namespace NFE {
namespace Common {
#define TLS thread_local static const

// Threads' debug name - global variable with implementation per thread
TLS std::string threadDebugName = 'T' + std::to_string(GetCurrentThreadId());
TLS unsigned int threadId = static_cast<unsigned int>(pthread_self());

bool Thread::SetThreadPriority(std::thread& thread, ThreadPriority priority)
{
    sched_param schParam;
    schParam.sched_priority = static_cast<int>(priority);
    auto res = pthread_setschedparam(threads.native_handle(), SCHED_RR, &schParam);
    return (res == 0) ? true : false;
}

const unsigned int Thread::GetCurrentThreadID()
{
    return threadId;
}

const std::string& Thread::GetCurrentThreadName()
{
    return threadDebugName;
}

} // namespace Common
} // namespace NFE
