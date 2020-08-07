/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class definition.
 */

#include "PCH.hpp"
#include "../Thread.hpp"
#include "../Assertion.hpp"
#include "../../Logger/Logger.hpp"

#include <process.h>

namespace
{

static const DWORD SetThreadNameExceptionID = 0x406D1388;

struct ThreadNameInfo
{
    DWORD  dwType;     // Must be 0x1000.
    LPCSTR szName;     // Pointer to name (in user addr space).
    DWORD  dwThreadID; // Thread ID (-1=caller thread).
    DWORD  dwFlags;    // Reserved for future use, must be zero.
};

} // namespace

namespace NFE {
namespace Common {

Thread::ThreadData::ThreadData()
    : handle(INVALID_HANDLE_VALUE)
    , id(0)
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

bool Thread::RunFunction(std::function<void(void)>&& callback)
{
    Wait();

    if (!mThreadData)
    {
        mThreadData = MakeUniquePtr<ThreadData>();
    }

    mThreadData->callback = std::move(callback);
    mThreadData->handle = (HANDLE)_beginthreadex(nullptr, 0, &Thread::ThreadCallback, mThreadData.Get(), CREATE_SUSPENDED, nullptr);

    if (mThreadData->handle == 0)
    {
        const int errorCode = errno;
        NFE_LOG_ERROR("Failed to create thread, errno=%d", errorCode);
        mThreadData->handle = INVALID_HANDLE_VALUE;
        return false;
    }

    mThreadData->id = ::GetThreadId(mThreadData->handle);
    NFE_ASSERT(0u != mThreadData->id, "Invalid thread ID returned by GetThreadId");
    NFE_LOG_DEBUG("Created thread with ID %0X", mThreadData->id);

    ::ResumeThread(mThreadData->handle);

    return true;
}

uint32 __stdcall Thread::ThreadCallback(void* arg)
{
    const ThreadData* threadData = reinterpret_cast<const ThreadData*>(arg);

    threadData->callback();

    NFE_LOG_DEBUG("Thread %0X finished", threadData->id);

    return 0;
}

void Thread::Wait()
{
    if (!mThreadData || mThreadData->handle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    ::SetLastError(0);
    if (WAIT_OBJECT_0 != ::WaitForSingleObject(mThreadData->handle, INFINITE))
    {
        const DWORD errorCode = ::GetLastError();
        NFE_LOG_ERROR("Waiting for thread %0X failed. Error code: %u", mThreadData->id, errorCode);
        return;
    }

    ::CloseHandle(mThreadData->handle);

    mThreadData->handle = INVALID_HANDLE_VALUE;
    mThreadData->id = 0;
}

uint32 Thread::GetID() const
{
    if (mThreadData)
    {
        return mThreadData->id;
    }

    return 0;
}

static DWORD gMainThreadID = ::GetCurrentThreadId();

bool Thread::IsMainThread()
{
    // TODO handle this better
    NFE_ASSERT(0u != gMainThreadID, "Main thread ID was not initialized, cannot tell if called from main thread or not");

    return gMainThreadID == ::GetCurrentThreadId();
}

bool Thread::SetPriority(ThreadPriority priority)
{
    if (!mThreadData || mThreadData->handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    ::SetLastError(0);
    if (0 != ::SetThreadPriority(mThreadData->handle, static_cast<int>(priority)))
    {
        return true;
    }

    const DWORD errorCode = ::GetLastError();
    NFE_LOG_ERROR("Failed to change thread %0X priority. Error code: %u", mThreadData->id, errorCode);
    return false;
}

bool Thread::SetCurrentThreadPriority(ThreadPriority priority)
{
    ::SetLastError(0);
    if (0 != ::SetThreadPriority(GetCurrentThread(), static_cast<int>(priority)))
    {
        return true;
    }

    const DWORD errorCode = ::GetLastError();
    NFE_LOG_ERROR("Failed to change thread's priority. Error code: %u", errorCode);
    return false;
}

bool Thread::SetName(const char* name)
{
    if (!mThreadData || mThreadData->id == 0)
    {
        return false;
    }

    ::ThreadNameInfo info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = mThreadData->id;
    info.dwFlags = 0;

    __try
    {
        RaiseException(SetThreadNameExceptionID, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}

    return true;
}

bool Thread::SetCurrentThreadName(const char* name)
{
    ::ThreadNameInfo info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = static_cast<DWORD>(-1);
    info.dwFlags = 0;

    __try
    {
        RaiseException(SetThreadNameExceptionID, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { }

    return true;
}

uint32 Thread::GetCurrentThreadID()
{
    return ::GetCurrentThreadId();
}

uint32 Thread::GetSystemThreadsCount()
{
    SYSTEM_INFO systemInfo = {};
    GetSystemInfo(&systemInfo);

    return systemInfo.dwNumberOfProcessors;
}

void Thread::YieldCurrentThread()
{
    ::SwitchToThread();
}

void Thread::SleepCurrentThread(double seconds)
{
    NFE_ASSERT(seconds > 0.0 && seconds < 1000000.0, "Invalid time specified");

    // TODO beter precission if needed
    ::Sleep((uint32)ceil(1000.0 * seconds));
}

} // namespace Common
} // namespace NFE
