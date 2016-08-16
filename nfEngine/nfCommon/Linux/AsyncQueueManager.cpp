/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Windows implementation of AsyncQueueManager class
 */

#include "../PCH.hpp"
#include "../AsyncQueueManager.hpp"
#include "../Logger.hpp"

#include <unistd.h>     /* for syscall() */
#include <poll.h>
#include <sys/syscall.h>    /* for __NR_* definitions */

#define NUM_EVENTS 128

namespace {
#ifndef __NR_eventfd
#if defined(__x86_64__)
#define __NR_eventfd 284
#elif defined(__i386__)
#define __NR_eventfd 323
#else
#error Cannot detect your OS architecture!
#endif
#endif

NFE_INLINE long io_setup(unsigned nr_reqs, aio_context_t *ctx)
{
    return syscall(__NR_io_setup, nr_reqs, ctx);
}

NFE_INLINE int io_destroy(aio_context_t ctx) 
{
    return syscall(__NR_io_destroy, ctx);
}

bool setupIo(::aio_context_t& ctx)
{
    auto res = ::io_setup(NUM_EVENTS, &ctx);
    if (res < 0)
    {
        LOG_ERROR("FileAsync failed to setup aio_context[%u]: %s", errno, strerror(errno));
        return false;
    }
    return true;
}

} // namespace


namespace NFE {
namespace Common {

AsyncQueueManager::AsyncQueueManager()
    : mIsDestroyed(false)
{
    mQueueThread = std::thread(AsyncQueueManager::JobQueue);
    //mEventPollFD = epoll_create1(0);
    ::setupIo(mCtx);
}

AsyncQueueManager::~AsyncQueueManager()
{
    // Set destroy flag and wait for queue thread to decay
    mIsDestroyed = true;
    mQueueThread.join();

    // Clear events map
    mFdMap.clear();
    mDescriptors.clear();

    // Close context
    ::io_destroy(mCtx);
}

AsyncQueueManager& AsyncQueueManager::GetInstance()
{
    static AsyncQueueManager instance;
    return instance;
}

bool AsyncQueueManager::EnqueueJob(JobProcedure& callback, int eventFD, void* data)
{
    //struct ::epoll_event event;
    //event.events = ::EPOLLIN;
    //event.data.fd = FD;
    //event.data.ptr = data;
    //int epollAction = EPOLL_CTL_ADD;
    if (auto it = mFdMap.find(eventFD) != mFdMap.end())
    {
        mFDMap.erase(it);
        for (auto &i: mDescriptors)
            if (i.fd == eventFD)
            {
                mDescriptors.erase(i);
                break;
            }
        return false;
    } else
    {
        mDescriptors.push_back({eventFD, POLLIN, 0});
        mFDMap[eventFD] = callback;
        return true;
    }
    /*
    // If given FD is already watched, it will be removed
    if (mFdMap.find(FD) != mFdMap.end())
        epollAction = EPOLL_CTL_DEL;

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
*/
    
}

void AsyncQueueManager::JobQueue()
{
    const int pollTimeout = 500;  // poll() timeout in milliseconds
    //::io_event eventsPollBuffer[NUM_EVENTS];
    int waitingEvents = 0;
    AsyncQueueManager* instance = &AsyncQueueManager::GetInstance();
    //int i = 0;

    
    while (!instance->mIsDestroyed)
    {
        for (auto &i : mDescriptors)
            i.revents = 0;
        
        // Poll to check if there are any events we may be interested in
        waitingEvents = ::poll(&mDescriptors, mDescriptors.size(), pollTimeout);
        if (waitingEvents < 0) // Error
        {
            LOG_ERROR("poll() for AsyncQueueManager failed: %s", strerror(errno));
            break;
        }
        else if (waitingEvents == 0) // Timeout
            continue;

        for (const auto&i : mDescriptors)
            if (i.revents & POLLIN)
            {
                JobProcedure jobFunc = mFDMap[i.fd];
                jobFunc(waitingEvents);
            }
          /*  
        // If waitingEvents > 0, we should get completed events from completion queue
        for (const auto& i : eventsPollBuffer)
        {
            // Check if the event has the right type
            if (i.events & EPOLLIN)
            {
                // Check if it's one of the events, we look for
                auto it = instance->mFdMap.find(i.data.fd);

                // If so, call it's callback to further process events
                if (it != instance->mFdMap.end())
                {
                    JobProcedure jobFunc = it->second;
                    jobFunc(i.data.ptr);
                }
            }
        }*/
    }
}

const std::thread& AsyncQueueManager::GetQueueThread() const
{
    return mQueueThread;
}

const aio_context_t AsyncQueueManager::GetQueueContext() const
{
    return mCtx;
}

} // namespace Common
} // namespace NFE
