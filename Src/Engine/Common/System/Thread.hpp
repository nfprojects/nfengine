/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class declaration.
 */

#pragma once
#include "../nfCommon.hpp"
#include "../Containers/UniquePtr.hpp"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(WIN32)

#include <functional>


namespace NFE {
namespace Common {

#if defined(__LINUX__) | defined(__linux__)
enum class ThreadPriority
{
    Idle,
    Lowest,
    BelowNormal,
    Normal,
    AboveNormal,
    Highest,
    RealTime,
};
#elif defined(WIN32)
enum class ThreadPriority
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

#define MAX_THREAD_NAME_LENGTH 16

/**
 * Helper methods for thread interaction
 */
class NFCOMMON_API Thread
{
    NFE_MAKE_NONCOPYABLE(Thread)

public:

    using CallbackFunc = std::function<void(void)>;

    Thread();
    ~Thread();

    Thread(Thread&& other);
    Thread& operator = (Thread&& other);

    template<typename Func, typename... Args>
    bool Run(Func&& func, Args&&... args)
    {
        return RunFunction(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    }

    bool RunFunction(CallbackFunc&& callback);

    // Wait until the thread finishes
    void Wait();

    // Set thread's priority
    bool SetPriority(ThreadPriority priority);

    // Set thread's name
    // Name can be MAX_THREAD_NAME_LENGTH characters long at max (including '\0')
    bool SetName(const char* name);

    // Get thread's ID
    uint32 GetID() const;

    // Check if the funcion is called from the main thread
    static bool IsMainThread();

    // Set current thread's priority
    static bool SetCurrentThreadPriority(ThreadPriority priority);

    // Set current thread's name
    // Name can be MAX_THREAD_NAME_LENGTH characters long at max (including '\0')
    static bool SetCurrentThreadName(const char* name);

    // Get current thread's ID
    static uint32 GetCurrentThreadID();

    // Get number of system threads (logical cores).
    static uint32 GetSystemThreadsCount();

    static void YieldCurrentThread();
    static void SleepCurrentThread(double seconds);

private:

    enum class State
    {
        Closed,
        Creating,
        Running,
        Closing,
    };

    // this wraps thread state, so Thread objects can be moved safely
    struct ThreadData
    {
#if defined(WIN32)
        HANDLE handle;
        DWORD id;
#elif defined(__LINUX__) | defined(__linux__)
        pthread_t id;
        uint32 uniqueId;
#endif // defined(WIN32)

        CallbackFunc callback;

        ThreadData();
    };

    UniquePtr<ThreadData> mThreadData;

#if defined(WIN32)
    static uint32 __stdcall ThreadCallback(void* arg);
#elif defined(__LINUX__) | defined(__linux__)
    static void* ThreadCallback(void* arg);
#endif // defined(WIN32)

};

} // namespace Common
} // namespace NFE
