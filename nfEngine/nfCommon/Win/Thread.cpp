/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class definition.
 */

#include "../PCH.hpp"
#include "../Thread.hpp"


namespace
{

const DWORD MS_VC_EXCEPTION = 0x406D1388;

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


thread_local const size_t threadId = static_cast<size_t>(::GetCurrentThreadId());

bool Thread::SetThreadPriority(std::thread& thread, ThreadPriority priority)
{
    auto result = ::SetThreadPriority(thread.native_handle(), static_cast<int>(priority));
    return (result != 0);
}

bool Thread::SetCurrentThreadName(const char* name)
{
    if (std::char_traits<char>::length(name) >= MAX_THREAD_NAME_LENGTH)
        return false;

    ::ThreadNameInfo info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = static_cast<DWORD>(-1);
    info.dwFlags = 0;

    __try
    {
        RaiseException(::MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR),
                       reinterpret_cast<ULONG_PTR*>(&info));
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return true;
}

size_t Thread::GetCurrentThreadId()
{
    return threadId;
}

} // namespace Common
} // namespace NFE
