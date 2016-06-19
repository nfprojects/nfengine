/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class definition.
 */

#include "PCH.hpp"
#include "Thread.hpp"
#include <thread>

#if defined(__LINUX__) | defined(__linux__)
#include <pthread.h>
#endif // defined(__LINUX__) | defined(__linux__)


namespace NFE {
namespace Common {

#if defined(__LINUX__) | defined(__linux__)
enum class threadPriority
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
enum class threadPriority
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


bool Thread::SetThreadPriority(std::thread& thread, threadPriority priority)
{
#if defined(WIN32)
    return ::SetThreadPriority(thread.native_handle() , static_cast<int>(priority)) != 0;
#elif defined(__LINUX__) | defined(__linux__)
    sched_param schedParam;
    schedParam.sched_priority = static_cast<int>(priority);
    int res = pthread_setschedparam(thread.native_handle(), SCHED_RR, &schedParam);
    return (res == 0) ? true : false;
#endif // defined(WIN32)
}

unsigned  int Thread::GetCurrentThreadID()
{
#ifdef WIN32
    return static_cast<unsigned int>(GetCurrentThreadId());
#elif defined(__LINUX__) | defined(__linux__)
    return static_cast<unsigned int>(pthread_self());
#endif // defined(WIN32)
}

} // namespace Common
} // namespace NFE
