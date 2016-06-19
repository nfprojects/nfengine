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

thread_local const unsigned int threadId = static_cast<unsigned int>(pthread_self());
thread_local const std::string threadDebugName = 'T' + std::to_string(GetCurrentThreadId());

bool Thread::SetThreadPriority(std::thread& thread, ThreadPriority priority)
{
    sched_param schParam;
    schParam.sched_priority = static_cast<int>(priority);
    auto res = pthread_setschedparam(threads.native_handle(), SCHED_RR, &schParam);
    return (res == 0) ? true : false;
}

} // namespace Common
} // namespace NFE
