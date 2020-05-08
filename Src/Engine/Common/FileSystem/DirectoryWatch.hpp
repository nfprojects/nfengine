/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  DirectoryWatch utility declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/Mutex.hpp"
#include "../Containers/UniquePtr.hpp"
#include "../Containers/HashMap.hpp"
#include "../Containers/DynArray.hpp"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "../System/Windows/Common.hpp"

#endif

#include <memory>
#include <type_traits>
#include <atomic>
#include <thread>


namespace NFE {
namespace Common {

#if defined(WIN32)
struct WatchRequest
{
    static constexpr uint32 BufferSize = 16384;

    int filter;
    DirectoryWatch* watch;

    String path;
    Utf16String widePath;

    HANDLE dirHandle;
    OVERLAPPED overlapped;
    DynArray<char> frontBuffer;
    DynArray<char> backBuffer;

    WatchRequest();

    bool Start();
    void Stop();
};
#endif // defined(WIN32)

/**
 * Class which allows user to watch directories for changes.
 */
class NFCOMMON_API DirectoryWatch
{
    NFE_MAKE_NONCOPYABLE(DirectoryWatch)
    NFE_MAKE_NONMOVEABLE(DirectoryWatch)

private:
#if defined(WIN32)

    friend struct WatchRequest;

    HANDLE mEvent;
    HANDLE mThread;

    Mutex mMutex;
    std::atomic<size_t> mRequestsNum;
    HashMap<String, UniquePtr<WatchRequest>> mRequests;

    static DWORD CALLBACK Dispatcher(LPVOID param);
    static void CALLBACK NotificationCompletion(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped);
    static void CALLBACK AddDirectoryProc(ULONG_PTR arg);
    static void CALLBACK RemoveDirectoryProc(ULONG_PTR arg);
    static void CALLBACK TerminateProc(ULONG_PTR arg);

#elif defined(__LINUX__) | defined(__linux__)
    std::thread mWatchThread;  //< worker thread
    int inotifyFd;             //< inotify file descriptor

    HashMap<String, int> mWatchPathMap;
    Mutex mWatchDescriptorMapMutex; //< lock for mWatchDescriptorMap
    HashMap<int, String> mWatchDescriptorMap;

    void WatchRoutine();
    bool ProcessInotifyEvent(void* event);
#else
#error "Target system not supported!"
#endif

public:
    // notification event type
    enum class Event : uint8
    {
        None     = 0,
        Create   = 1 << 0,  //< a file or a directory has been created
        Delete   = 1 << 1,  //< a file or a directory has been deleted
        Modify   = 1 << 2,  //< a file has been modified
        MoveFrom = 1 << 3,  //< a file has been moved from
        MoveTo   = 1 << 4,  //< a file has been moved to
    };

    using EventType = std::underlying_type<Event>::type;

    // notification event data
    struct EventData
    {
        Event type;           //< event type
        const char* path;     //< a file or a directory that the event applies to
        EventData() : type(Event::None), path(nullptr) {}
    };

    /**
     * Callback function for watched directory event.
     * The function should be as lightweight as possible, because other events
     * are handled on the same thread.
     */
    using WatchCallback = std::function<void(const EventData& event)>;

    DirectoryWatch();
    ~DirectoryWatch();

    /**
     * Adds, modifies or removes watch for a specified path.
     * This function is NOT thread-safe.
     *
     * @param  path        Watch path to be modified.
     * @param  eventFilter Bitfield selecting event types to be monitored.
     * @return True on success
     */
    bool WatchPath(const String& path, Event eventFilter);

    /**
     * Change directory monitor callback.
     * The callback function will be called from an internal worker thread.
     *
     * @param callback New callback function.
     */
    void SetCallback(WatchCallback callback);

private:
    std::atomic<bool> mRunning; //< is directory watch running?
    WatchCallback mCallback;
};


NFE_INLINE DirectoryWatch::Event operator&(DirectoryWatch::Event a, DirectoryWatch::Event b)
{
    return static_cast<DirectoryWatch::Event>(static_cast<DirectoryWatch::EventType>(a) &
                                              static_cast<DirectoryWatch::EventType>(b));
}

NFE_INLINE DirectoryWatch::Event operator|(DirectoryWatch::Event a, DirectoryWatch::Event b)
{
    return static_cast<DirectoryWatch::Event>(static_cast<DirectoryWatch::EventType>(a) |
                                              static_cast<DirectoryWatch::EventType>(b));
}

NFE_INLINE DirectoryWatch::Event& operator|=(DirectoryWatch::Event& lhs, DirectoryWatch::Event rhs)
{
    lhs = static_cast<DirectoryWatch::Event>(static_cast<DirectoryWatch::EventType>(lhs) |
                                             static_cast<DirectoryWatch::EventType>(rhs));
    return lhs;
}

} // namespace Common
} // namespace NFE
