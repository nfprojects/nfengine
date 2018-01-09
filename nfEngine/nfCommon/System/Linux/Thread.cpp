/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class definition.
 */

#include "PCH.hpp"
#include "../Thread.hpp"
#include "Logger/Logger.hpp"

#include <pthread.h>
#include <string.h>


namespace NFE {
namespace Common {

thread_local const size_t threadId = static_cast<size_t>(pthread_self());

bool Thread::SetThreadPriority(std::thread& thread, ThreadPriority priority)
{
    // On linux we need SU privilages to change threads priority or policy.
    // Only change that is possible with normal privilages would be changing policy
    // between normal (OTHER) and idle or batch - we don't need those.
    NFE_UNUSED(thread);
    NFE_UNUSED(priority);
    LOG_WARNING("SetThreadPriority method does nothing on this system.");

    return false;
}

bool Thread::SetCurrentThreadName(const char* name)
{
    if (std::char_traits<char>::length(name) < MAX_THREAD_NAME_LENGTH)
    {    
        auto retVal = pthread_setname_np(static_cast<pthread_t>(threadId), name);
        if(retVal == 0)
            return true;
        else
            NFE_LOG_ERROR("Error while setting threads name to '%s': %s", name, strerror(retVal));
    } else
        NFE_LOG_ERROR("Thread name too long - including \\0 it cannot be longer than 16 chars");

    return false;
}

size_t Thread::GetCurrentThreadId()
{
    return threadId;
}

} // namespace Common
} // namespace NFE
