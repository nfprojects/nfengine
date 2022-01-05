/**
 * @file
 * @author mkulagowski, Witek902
 * @brief  Thread class definition.
 */

#include "PCH.hpp"
#include "../Thread.hpp"
#include "../Assertion.hpp"
#include "../../Logger/Logger.hpp"

#include <pthread.h>
#include <sys/sysinfo.h>


namespace NFE {
namespace Common {

static const pthread_t gMainThreadId = pthread_self();
static const bool gMainThreadIdInitialized = true;
static std::atomic<uint32> gLastThreadId = 0u;
thread_local uint32 gCurrentThreadUniqueId = 0u;

Thread::ThreadData::ThreadData()
    : id(0)
    , uniqueId(UINT32_MAX)
{
}

Thread::Thread() = default;

Thread::~Thread()
{
    Wait();
}

Thread::Thread(Thread&& other)
    : mThreadData(std::move(other.mThreadData))
{
    NFE_ASSERT(other.mThreadData.Get() == nullptr, "Thread data was not moved properly");
}

Thread& Thread::operator = (Thread&& other)
{
    if (this != &other)
    {
        Wait();
        mThreadData = std::move(other.mThreadData);
    }

    return *this;
}

bool Thread::RunFunction(CallbackFunc&& callback)
{
    Wait();

    if (!mThreadData)
    {
        mThreadData = MakeUniquePtr<ThreadData>();
    }

    mThreadData->callback = std::move(callback);
    mThreadData->uniqueId = ++gLastThreadId;

    const pthread_attr_t *attr = nullptr;
    if (0 != pthread_create(&mThreadData->id, attr, &Thread::ThreadCallback, mThreadData.Get()))
    {
        const int errorCode = errno;
        NFE_LOG_ERROR("Failed to create thread, errno=%d", errorCode);
        mThreadData->callback = CallbackFunc();
        return false;
    }

    return true;
}

void* Thread::ThreadCallback(void* arg)
{
    const ThreadData* threadData = reinterpret_cast<const ThreadData*>(arg);

    gCurrentThreadUniqueId = threadData->uniqueId;

    threadData->callback();

    NFE_LOG_DEBUG("Thread %u finished", threadData->uniqueId);

    return nullptr;
}

void Thread::Wait()
{
    if (!mThreadData || !mThreadData->callback)
    {
        return;
    }

    void* returnValue = nullptr;
    if (0 != pthread_join(mThreadData->id, &returnValue))
    {
        const int errorCode = errno;
        NFE_LOG_ERROR("Failed to wait for a thread, errno=%d", errorCode);
    }

    mThreadData->callback = CallbackFunc();
    mThreadData->id = 0;
    mThreadData->uniqueId = UINT32_MAX;
}

uint32 Thread::GetID() const
{
    if (mThreadData)
    {
        return mThreadData->uniqueId;
    }

    return UINT32_MAX;
}

bool Thread::IsMainThread()
{
    // TODO handle this better
    NFE_ASSERT(gMainThreadIdInitialized, "Main thread ID was not initialized, cannot tell if called from main thread or not");

    return 0 != pthread_equal(gMainThreadId, pthread_self());
}

bool Thread::SetPriority(ThreadPriority priority)
{
    // On linux we need SU privilages to change threads priority or policy.
    // Only change that is possible with normal privilages would be changing policy
    // between normal (OTHER) and idle or batch - we don't need those.
    NFE_UNUSED(priority);
    NFE_LOG_WARNING("SetThreadPriority method does nothing on this system.");

    return false;
}

bool Thread::SetCurrentThreadPriority(ThreadPriority priority)
{
    // On linux we need SU privilages to change threads priority or policy.
    // Only change that is possible with normal privilages would be changing policy
    // between normal (OTHER) and idle or batch - we don't need those.
    NFE_UNUSED(priority);
    NFE_LOG_WARNING("SetThreadPriority method does nothing on this system.");

    return false;
}

bool Thread::SetName(const char* name)
{
    if (!mThreadData || !mThreadData->callback)
    {
        return false;
    }

    // pthread requires thread name to have 16 chars max, including NULL
    char threadName[16];
    strncpy(threadName, name, 16);
    threadName[15] = 0;

    auto retVal = pthread_setname_np(mThreadData->id, threadName);
    if (retVal != 0)
    {
        NFE_LOG_ERROR("Error while setting threads name to '%s': %s", name, strerror(retVal));
        return false;
    }

    return true;
}

bool Thread::SetCurrentThreadName(const char* name)
{
    // pthread requires thread name to have 16 chars max, including NULL
    char threadName[16];
    strncpy(threadName, name, 16);
    threadName[15] = 0;

    auto retVal = pthread_setname_np(pthread_self(), name);
    if (retVal != 0)
    {
        NFE_LOG_ERROR("Error while setting threads name to '%s': %s", name, strerror(retVal));
        return false;
    }

    return true;
}

uint32 Thread::GetCurrentThreadID()
{
    return gCurrentThreadUniqueId;
}

uint32 Thread::GetSystemThreadsCount()
{
    return (uint32)get_nprocs();
}

void Thread::YieldCurrentThread()
{
    pthread_yield();
}

void Thread::SleepCurrentThread(double seconds)
{
    NFE_ASSERT(seconds > 0.0, "Invalid time specified");

    struct timespec t;
    t.tv_sec = (uint32)seconds;
    t.tv_nsec = (uint32)(1000000000.0 * fmod(seconds, 1.0));

    if (0 != nanosleep(&t , nullptr))
    {
        const int errorCode = errno;
        NFE_LOG_ERROR("nanosleep() failed, t = %f seconds, error code: %u", seconds, errorCode);
    }
}

} // namespace Common
} // namespace NFE
