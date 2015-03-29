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
    if (inotifyFd != -1)
        ::close(inotifyFd);

    mWatchThread.join();
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
        mWatchDescriptorMap.erase(fd);
        return true;
    }

    uint32_t inotifyMask = IN_ONLYDIR;

    if ((eventFilter & Event::Create) == Event::Create)
        inotifyMask |= IN_CREATE;
    if ((eventFilter & Event::Delete) == Event::Delete)
        inotifyMask |= IN_DELETE | IN_DELETE_SELF;
    if ((eventFilter & Event::MoveFrom) == Event::MoveFrom)
        inotifyMask |= IN_MOVED_FROM | IN_MOVE_SELF;
    if ((eventFilter & Event::MoveTo) == Event::MoveTo)
        inotifyMask |= IN_MOVED_TO;
    if ((eventFilter & Event::Modify) == Event::Modify)
        inotifyMask |= IN_CLOSE_WRITE;

    int fd = ::inotify_add_watch(inotifyFd, path.c_str(), inotifyMask);
    if (fd < 0)
    {
        LOG_ERROR("inotify_add_watch() failed: %s", strerror(errno));
        return false;
    }

    int oldFd = mWatchPathMap[path];
    mWatchDescriptorMap.erase(oldFd);
    mWatchPathMap[path] = fd;
    mWatchDescriptorMap[fd] = path;
    LOG_DEBUG("Directory watch for path '%s' added, wd = %d", path.c_str(), fd);
    return true;
}

#define MAX_BUFFERED_EVENTS 32
#define EVENT_BUFFER_LEN (MAX_BUFFERED_EVENTS * (sizeof(struct ::inotify_event) + NAME_MAX + 1))

void DirectoryWatch::WatchRoutine()
{
    // poll() timeout in milliseconds
    const int pollTimeout = 50;
    char buffer[EVENT_BUFFER_LEN];
    ssize_t numRead;
    struct ::inotify_event* event;

    for (;;)
    {
        /*
         * Wait for inotify events.
         * When no events are availavle and DirectoryWatch destructor is called,
         * read() would wait forever.
         */
        struct ::pollfd pfd = {inotifyFd, POLLIN, 0};
        int ret = ::poll(&pfd, 1, pollTimeout);
        if (ret < 0)
        {
            LOG_ERROR("poll() for inotify failed: %s", strerror(errno));
            break;
        }
        else if (ret == 0)
            continue;

        // read inotify events
        numRead = ::read(inotifyFd, buffer, EVENT_BUFFER_LEN);

        if (numRead == 0)
        {
            LOG_ERROR("read() for inotify returned 0");
            break;
        }

        if (numRead < 0)
            break;

        for (char *p = buffer; p < buffer + numRead;)
        {
            event = reinterpret_cast<struct ::inotify_event *>(p);
            ProcessInotifyEvent(event);
            p += sizeof(struct ::inotify_event); // event header
            p += event->len;  // path
        }
    }

    if (numRead < 0 && mRunning)
        LOG_ERROR("read() for inotify failed: %s", strerror(errno));
    else
        LOG_DEBUG("WatchRoutine() was interrupted");
}

bool DirectoryWatch::ProcessInotifyEvent(void* event)
{
    EventData eventData;
    eventData.type = Event::None;

    struct ::inotify_event* e = reinterpret_cast<struct ::inotify_event*>(event);

    char path[PATH_MAX];
    strcpy(path, mWatchDescriptorMap[e->wd].c_str());

    if (e->mask & IN_CREATE)
    {
        strcat(path, "/");
        strcat(path, e->name);
        eventData.path = path;
        eventData.type |= Event::Create;
        LOG_DEBUG("DirectoryWatch: path created, path = %s", path);
    }
    else if (e->mask & IN_DELETE)
    {
        strcat(path, "/");
        strcat(path, e->name);
        eventData.path = path;
        eventData.type |= Event::Delete;
        LOG_DEBUG("DirectoryWatch: path deleted, path = %s", path);
    }
    else if (e->mask & IN_CLOSE_WRITE)
    {
        strcat(path, "/");
        strcat(path, e->name);
        eventData.path = path;
        eventData.type |= Event::Modify;
        LOG_DEBUG("DirectoryWatch: file modified, path = %s", path);
    }
    else if (e->mask & IN_MOVED_FROM)
    {
        strcat(path, "/");
        strcat(path, e->name);
        eventData.path = path;
        eventData.type |= Event::MoveFrom;
        LOG_DEBUG("DirectoryWatch: file moved from, path = %s", path);
    }
    else if (e->mask & IN_MOVED_TO)
    {
        strcat(path, "/");
        strcat(path, e->name);
        eventData.path = path;
        eventData.type |= Event::MoveTo;
        LOG_DEBUG("DirectoryWatch: file moved to, path = %s", path);
    }
    else if (e->mask & IN_DELETE_SELF)
    {
        eventData.path = path;
        eventData.type |= Event::Delete;
        LOG_DEBUG("DirectoryWatch: watched path deleted, path = %s", path);
    }
    else if (e->mask & IN_MOVE_SELF)
    {
        eventData.path = path;
        eventData.type |= Event::MoveFrom;
        LOG_DEBUG("DirectoryWatch: watched path moved, path = %s", path);
    }

    if (eventData.type != Event::None)
        if (mCallback)
            mCallback(eventData);

    return true;
}

} // namespace Common
} // namespace NFE
