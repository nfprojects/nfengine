/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  DirectoryWatch utility declarations.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

/**
 * Class allowing watching directories changes.
 */
class DirectoryWatch
{
private:
#if defined(WIN32)
    // TODO
#elif defined(__LINUX__) | defined(__linux__)
    int inotifyFd; //< inotify file descriptor

    // TODO: replace with single container?
    std::map<std::string, int> mWatchPathMap;
    std::map<int, std::string> mWatchDescriptorMap;

    // for tracing IN_MOVED_FROM and IN_MOVED_TO events
    std::map<uint32_t, std::string> mCookieMap;
    bool ProcessInotifyEvent(void* event);
#else
#error "Target system not supported!"
#endif

public:
    enum class Event : int
    {
        None   = 0,
        Create = 1 << 0,  //< a file or a directory has been created
        Delete = 1 << 1,  //< a file or a directory has been deleted
        Move   = 1 << 2,  //< a file or a directory has been moved (renamed)
        Modify = 1 << 3,  //< a file has been modified
    };

    struct EventData
    {
        Event type;           //< event type
        const char* path;     //< a file or a directory that the event applies to
        const char* newPath;  //< new file or directory name (filled only for "Move" event type)

        EventData() : type(Event::None), path(nullptr), newPath(nullptr) {}
    };

    typedef std::function<void(const EventData& event)> WatchCallback;

    DirectoryWatch();
    ~DirectoryWatch();

    /**
     * Adds, modifies or removes watch for a specified path.
     * @param  path        Watch path to be modified.
     * @param  eventFilter Bitfield selecting event types to be monitored.
     * @return True on success
     */
    bool WatchPath(const std::string& path, Event eventFilter);

    /**
     * Change directory monitor callback.
     * @param callback New callback function.
     */
    void SetCallback(WatchCallback callback);

private:
    std::atomic<bool> mRunning;
    std::thread mWatchThread;
    WatchCallback mCallback;

    void WatchRoutine();

    /// disable these methods
    DirectoryWatch(const DirectoryWatch&) = delete;
    DirectoryWatch(DirectoryWatch&&) = delete;
    DirectoryWatch& operator=(const DirectoryWatch&) = delete;
    DirectoryWatch& operator=(DirectoryWatch&&) = delete;
};


inline DirectoryWatch::Event operator&(DirectoryWatch::Event a, DirectoryWatch::Event b)
{
    return static_cast<DirectoryWatch::Event>(static_cast<int>(a) & static_cast<int>(b));
}

inline DirectoryWatch::Event operator|(DirectoryWatch::Event a, DirectoryWatch::Event b)
{
    return static_cast<DirectoryWatch::Event>(static_cast<int>(a) | static_cast<int>(b));
}

inline DirectoryWatch::Event& operator |= (DirectoryWatch::Event& lhs, DirectoryWatch::Event rhs)
{
    lhs = static_cast<DirectoryWatch::Event>(static_cast<int>(lhs) | static_cast<int>(rhs));
    return lhs;
}

} // namespace Common
} // namespace NFE
