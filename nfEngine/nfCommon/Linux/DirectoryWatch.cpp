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

    uint32_t inotifyMask = 0;
    if ((eventFilter & Event::Create) == Event::Create)
        inotifyMask |= IN_CREATE;
    if ((eventFilter & Event::Delete) == Event::Delete)
        inotifyMask |= IN_DELETE | IN_DELETE_SELF;
    if ((eventFilter & Event::Move) == Event::Move)
        inotifyMask |= IN_MOVED_FROM | IN_MOVED_TO | IN_MOVE_SELF;
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

#define MAX_BUFFERED_EVENTS 16
#define EVENT_BUFFER_LEN (MAX_BUFFERED_EVENTS * (sizeof(struct ::inotify_event) + NAME_MAX + 1))

void DirectoryWatch::WatchRoutine()
{
    const int pollTimeout = 100;
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
            LOG_ERROR("poll() for inotify returned 0");
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

// TODO: remove
static void displayInotifyEvent(struct inotify_event *i)
{
    std::stringstream ss;
    ss << "wd=" << i->wd;
    if (i->cookie > 0) ss << " cookie=" << i->cookie;
    ss << " mask=";
    if (i->mask & IN_ACCESS)        ss << "IN_ACCESS,";
    if (i->mask & IN_ATTRIB)        ss << "IN_ATTRIB,";
    if (i->mask & IN_CLOSE_NOWRITE) ss << "IN_CLOSE_NOWRITE,";
    if (i->mask & IN_CLOSE_WRITE)   ss << "IN_CLOSE_WRITE,";
    if (i->mask & IN_CREATE)        ss << "IN_CREATE,";
    if (i->mask & IN_DELETE)        ss << "IN_DELETE,";
    if (i->mask & IN_DELETE_SELF)   ss << "IN_DELETE_SELF,";
    if (i->mask & IN_IGNORED)       ss << "IN_IGNORED,";
    if (i->mask & IN_ISDIR)         ss << "IN_ISDIR,";
    if (i->mask & IN_MODIFY)        ss << "IN_MODIFY,";
    if (i->mask & IN_MOVE_SELF)     ss << "IN_MOVE_SELF,";
    if (i->mask & IN_MOVED_FROM)    ss << "IN_MOVED_FROM,";
    if (i->mask & IN_MOVED_TO)      ss << "IN_MOVED_TO,";
    if (i->mask & IN_OPEN)          ss << "IN_OPEN,";
    if (i->mask & IN_Q_OVERFLOW)    ss << "IN_Q_OVERFLOW,";
    if (i->mask & IN_UNMOUNT)       ss << "IN_UNMOUNT,";
    if (i->len > 0) ss << " name=" << i->name;
    LOG_DEBUG(ss.str().c_str());
}

bool DirectoryWatch::ProcessInotifyEvent(void* event)
{
    EventData eventData;
    eventData.type = Event::None;

    struct ::inotify_event *e = reinterpret_cast<struct ::inotify_event*>(event);
    displayInotifyEvent(e);

    std::string parentDir = mWatchDescriptorMap[e->wd] + '/';

    if (e->mask & IN_CREATE)
    {
        std::string path = parentDir + e->name;
        eventData.path = path.c_str();
        eventData.type |= Event::Create;
        LOG_DEBUG("DirectoryWatch: path created, path = %s", eventData.path);
    }
    else if (e->mask & IN_DELETE)
    {
        std::string path = parentDir + e->name;
        eventData.path = path.c_str();
        eventData.type |= Event::Delete;
        LOG_DEBUG("DirectoryWatch: path deleted, path = %s", eventData.path);
    }
    else if (e->mask & IN_CLOSE_WRITE)
    {
        std::string path = parentDir + e->name;
        eventData.path = path.c_str();
        eventData.type |= Event::Modify;
        LOG_DEBUG("DirectoryWatch: file modified, path = %s", eventData.path);
        // TODO: file could be opened and closed without modification...
    }
    else if (e->mask & IN_MOVED_FROM)
    {
        mCookieMap[e->cookie] = parentDir + std::string(e->name);
    }
    else if (e->mask & IN_MOVED_TO)
    {
        auto it = mCookieMap.find(e->cookie);
        if (it != mCookieMap.end())
        {
            std::string newPath = parentDir + e->name;
            const char* srcPath = it->second.c_str();
            eventData.path = srcPath;
            eventData.newPath = newPath.c_str();
            eventData.type |= Event::Move;
            LOG_DEBUG("DirectoryWatch: path moved, srcPath = %s, destPath = %s",
                      eventData.path, eventData.newPath);
        }
        else
        {
            std::string path = parentDir + e->name;
            eventData.newPath = path.c_str();
            eventData.type |= Event::Move;
            LOG_DEBUG("DirectoryWatch: path created, path = %s", eventData.path);
        }
    }
    else if (e->mask & IN_MOVE_SELF)
    {
        LOG_DEBUG("DirectoryWatch: watched path moved");
        eventData.path = e->name;
        eventData.type |= Event::Move;
    }


    if (eventData.type != Event::None)
        if (mCallback)
            mCallback(eventData);

    return true;
}

} // namespace Common
} // namespace NFE
