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
 * Class which allows user to watch directories for changes.
 */
class DirectoryWatch
{
public:
    // notification event type
    enum class Event : unsigned int
    {
        None     = 0,
        Create   = 1 << 0,  //< a file or a directory has been created
        Delete   = 1 << 1,  //< a file or a directory has been deleted
        Modify   = 1 << 2,  //< a file has been modified
        MoveFrom = 1 << 3,  //< a file has been moved from
        MoveTo   = 1 << 4,  //< a file has been moved to
    };

    typedef std::underlying_type<Event>::type EventType;

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
    typedef std::function<void(const EventData& event)> WatchCallback;

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
    bool WatchPath(const std::string& path, Event eventFilter);

    /**
     * Change directory monitor callback.
     * The callback function will be called from an internal worker thread.
     *
     * @param callback New callback function.
     */
    void SetCallback(WatchCallback callback);

private:
#if defined(WIN32)
    // TODO
#elif defined(__LINUX__) | defined(__linux__)
    std::thread mWatchThread;  //< worker thread
    int inotifyFd;             //< inotify file descriptor

    std::map<std::string, int> mWatchPathMap;
    std::map<int, std::string> mWatchDescriptorMap;

    void WatchRoutine();
    bool ProcessInotifyEvent(void* event);
#else
#error "Target system not supported!"
#endif

    std::atomic<bool> mRunning; //< is directory watch running?
    WatchCallback mCallback;

    /// disable these methods
    DirectoryWatch(const DirectoryWatch&) = delete;
    DirectoryWatch(DirectoryWatch&&) = delete;
    DirectoryWatch& operator=(const DirectoryWatch&) = delete;
    DirectoryWatch& operator=(DirectoryWatch&&) = delete;
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
