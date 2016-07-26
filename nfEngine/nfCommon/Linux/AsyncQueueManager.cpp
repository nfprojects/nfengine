/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Windows implementation of AsyncQueueManager class
 */

#include "../PCH.hpp"
#include "../AsyncQueueManager.hpp"
#include "../Logger.hpp"
#include "Common.hpp"

#include <sys/epoll.h>


namespace NFE {
namespace Common {

AsyncQueueManager::AsyncQueueManager()
    : mIsDestroyed(false)
{
    mQueueThread = std::thread(AsyncQueueManager::JobQueue);
    mEventPollFD = epoll_create1(0);
}

AsyncQueueManager::~AsyncQueueManager()
{
    // Set destroy flag and wait for queue thread to decay
    mIsDestroyed = true;
    mQueueThread.join();

    // Delete all epoll events
    for (const auto i& : mFdMap)
        ::epoll_ctl(mEventPollFD, EPOLL_CTL_DEL, i.first(), nullptr);

    // Clear events map
    mFdMap.clear();

    // CLose epoll descriptor
    ::close(mEventPollFD);
}

AsyncQueueManager& AsyncQueueManager::GetInstance()
{
    static AsyncQueueManager instance;
    return instance;
}

bool AsyncQueueManager::EnqueueJob(JobProcedure& callback, int FD, void* data)
{
    struct ::epoll_event event;
    event.events = ::EPOLLIN;
    event.data.fd = FD;
    event.data.ptr = data;
    int epollAction = ::EPOLL_CTL_ADD;

    // If given FD is already watched, it will be removed
    if (mFdMap.find(FD) != mFdMap.end())
        epollAction = ::EPOLL_CTL_DEL;

    // Call epoll_ctl to add/remove descriptor
    if (0 != ::epoll_ctl(mEventPollFD, epollAction, FD, &event))
    {
        LOG_ERROR("epoll_ctl() failed for AsyncQueueManager: %s", strerror(errno));
        return false;
    }

    // If given FD was removed, remove it from mFdMap as well
    if (auto it = mFdMap.find(FD) != mFdMap.end())
        mFdMap.erase(it);
    else // Else add it to the mFdMap
        mFdMap[FD] = callback;

    return true;
}

void AsyncQueueManager::JobQueue()
{
    const int pollTimeout = 500;  // epoll_wait() timeout in milliseconds
    ::epoll_event eventsPollBuffer[NUM_EVENTS];
    int waitingEvents = 0;

    while (!instance->mIsDestroyed)
    {
        // Poll to check if there are any events we may be interested in
        waitingEvents = ::epoll_wait(mEventPollFD, eventsPollBuffer, NUM_EVENTS, pollTimeout);
        if (waitingEvents < 0) // Error
        {
            LOG_ERROR("epoll_wait() for AsyncQueueManager failed: %s", strerror(errno));
            break;
        }
        else if (waitingEvents == 0) // Timeout
            continue;

        // If waitingEvents > 0, we should get completed events from completion queue
        for (const auto& i : eventsPollBuffer)
        {
            // Check if the event has the right type
            if (i.events & EPOLLIN)
            {
                // Check if it's one of the events, we look for
                auto it = mFdMap.find(i.data.fd);

                // If so, call it's callback to further process events
                if (it != mFdMap.end())
                {
                    JobProcedure jobFunc = it.second();
                    jobFunc(i.data.ptr);
                }
            }
        }
    }
}
     
const std::thread& AsyncQueueManager::GetQueueThread()
{
    return mQueueThread;
}

} // namespace Common
} // namespace NFE
