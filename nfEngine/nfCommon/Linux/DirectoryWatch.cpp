/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Linux implementation of DirectoryWatch class
 */

#include "../PCH.hpp"
#include "../DirectoryWatch.hpp"
#include "../Logger.hpp"

#include <sstream>
#include <poll.h>
#include <sys/inotify.h>
#include <limits.h>

namespace NFE {
namespace Common {

DirectoryWatch::DirectoryWatch()
{
    inotifyFd = ::inotify_init();
    if (inotifyFd == -1)
        LOG_ERROR("inotify_init() failed: %s", strerror(errno));

    mRunning.store(true);
    mWatchThread = std::thread(&DirectoryWatch::WatchRoutine, this);
}

DirectoryWatch::~DirectoryWatch()
{
    mRunning.store(false);
    mWatchThread.join();

    if (inotifyFd != -1)
        ::close(inotifyFd);
}

void DirectoryWatch::SetCallback(WatchCallback callback)
{
    mCallback = callback;
}

bool DirectoryWatch::WatchPath(const std::string& path, Event eventFilter)
{
    // remove watch
    if (eventFilter == Event::None)
    {
        // already removed
        if (mWatchPathMap.count(path) == 0)
            return true;

        int fd = mWatchPathMap[path];
        ::inotify_rm_watch(inotifyFd, fd);
        mWatchPathMap.erase(path);

        std::unique_lock<std::mutex> lock(mWatchDescriptorMapMutex);
        mWatchDescriptorMap.erase(fd);
        return true;
    }

    auto hasEvent = [eventFilter](Event mask)
    {
         return (eventFilter & mask) == mask;
    };

    uint32_t inotifyMask = IN_ONLYDIR;
    if (hasEvent(Event::Create))   inotifyMask |= IN_CREATE;
    if (hasEvent(Event::Delete))   inotifyMask |= IN_DELETE | IN_DELETE_SELF;
    if (hasEvent(Event::MoveFrom)) inotifyMask |= IN_MOVED_FROM | IN_MOVE_SELF;
    if (hasEvent(Event::MoveTo))   inotifyMask |= IN_MOVED_TO;
    if (hasEvent(Event::Modify))   inotifyMask |= IN_CLOSE_WRITE;

    int fd = ::inotify_add_watch(inotifyFd, path.c_str(), inotifyMask);
    if (fd < 0)
    {
        LOG_ERROR("inotify_add_watch() for path '%s' failed: %s", path.c_str(), strerror(errno));
        return false;
    }

    int oldFd = mWatchPathMap[path];
    {
        std::unique_lock<std::mutex> lock(mWatchDescriptorMapMutex);
        mWatchDescriptorMap.erase(oldFd);
        mWatchPathMap[path] = fd;
        mWatchDescriptorMap[fd] = path;
    }
    LOG_DEBUG("Directory watch for path '%s' added, wd = %d", path.c_str(), fd);
    return true;
}

#define MAX_BUFFERED_EVENTS 32
#define EVENT_BUFFER_LEN (MAX_BUFFERED_EVENTS * (sizeof(struct ::inotify_event) + NAME_MAX + 1))

void DirectoryWatch::WatchRoutine()
{
    const int pollTimeout = 40;  // poll() timeout in milliseconds
    char buffer[EVENT_BUFFER_LEN];

    while (mRunning)
    {
        /*
         * Wait for inotify events.
         *
         * When no events are availavle and DirectoryWatch destructor is called,
         * read() would wait forever - that's why we need to poll to check if
         * there are any events.
         */
        struct ::pollfd pfd = {inotifyFd, POLLIN, 0};
        int ret = ::poll(&pfd, 1, pollTimeout);
        if (ret < 0)
        {
            LOG_ERROR("poll() for inotify failed: %s", strerror(errno));
            break;
        }
        else if (ret == 0) // timeout
            continue;

        // read inotify events
        ssize_t numRead = ::read(inotifyFd, buffer, EVENT_BUFFER_LEN);
        if (numRead < 0)
        {
            if (mRunning)
                LOG_ERROR("read() for inotify failed: %s", strerror(errno));
            break;
        }
        else if (numRead == 0)
        {
            // this should not happen
            LOG_WARNING("read() for inotify returned 0");
            continue;
        }

        // process events from the received buffer
        for (char *p = buffer; p < buffer + numRead;)
        {
            struct ::inotify_event* event = reinterpret_cast<struct ::inotify_event *>(p);
            ProcessInotifyEvent(event);
            p += sizeof(struct ::inotify_event); // event header
            p += event->len;  // path
        }
    }
}

bool DirectoryWatch::ProcessInotifyEvent(void* event)
{
    struct ::inotify_event* e = reinterpret_cast<struct ::inotify_event*>(event);

    char path[PATH_MAX];
    {
        std::unique_lock<std::mutex> lock(mWatchDescriptorMapMutex);
        strcpy(path, mWatchDescriptorMap[e->wd].c_str());
    }

    EventData eventData;
    eventData.type = Event::None;
    eventData.path = path;

    auto constructEventData = [&](bool includeSubpath, Event eventType)
    {
        if (includeSubpath)
        {
            strcat(path, "/");
            strcat(path, e->name);
        }
        eventData.type |= eventType;
    };

    if (e->mask & IN_CREATE)
    {
        constructEventData(true, Event::Create);
        LOG_DEBUG("DirectoryWatch: path created, path = %s", path);
    }
    else if (e->mask & IN_DELETE)
    {
        constructEventData(true, Event::Delete);
        LOG_DEBUG("DirectoryWatch: path deleted, path = %s", path);
    }
    else if (e->mask & IN_CLOSE_WRITE)
    {
        constructEventData(true, Event::Modify);
        LOG_DEBUG("DirectoryWatch: file modified, path = %s", path);
    }
    else if (e->mask & IN_MOVED_FROM)
    {
        constructEventData(true, Event::MoveFrom);
        LOG_DEBUG("DirectoryWatch: file moved from, path = %s", path);
    }
    else if (e->mask & IN_MOVED_TO)
    {
        constructEventData(true, Event::MoveTo);
        LOG_DEBUG("DirectoryWatch: file moved to, path = %s", path);
    }
    else if (e->mask & IN_DELETE_SELF)
    {
        constructEventData(false, Event::Delete);
        LOG_DEBUG("DirectoryWatch: watched path deleted, path = %s", path);
    }
    else if (e->mask & IN_MOVE_SELF)
    {
        constructEventData(false, Event::MoveFrom);
        LOG_DEBUG("DirectoryWatch: watched path moved, path = %s", path);
    }

    if (eventData.type != Event::None)
        if (mCallback)
            mCallback(eventData);

    return true;
}

} // namespace Common
} // namespace NFE
