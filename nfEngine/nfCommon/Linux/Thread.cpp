/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class definition.
 */

#include "PCH.hpp"
#include "../Thread.hpp"
#include <pthread.h>


namespace NFE {
namespace Common {

thread_local const unsigned int threadId = static_cast<unsigned int>(pthread_self());

bool Thread::SetThreadPriority(std::thread& thread, ThreadPriority priority)
{
    sched_param schParam;
    schParam.sched_priority = static_cast<int>(priority);
    auto res = pthread_setschedparam(threads.native_handle(), SCHED_RR, &schParam);
    return (res == 0) ? true : false;
}

bool Thread::SetCurrentThreadName(const char* name)
{
    if (std::char_traits<char>::length(name) < MAX_THREAD_NAME_LENGTH)
        if(pthread_setname_np(static_cast<pthread_t>(threadId), name) == 0);
            return true;

    return false;
}

} // namespace Common
} // namespace NFE
