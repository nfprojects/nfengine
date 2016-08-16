/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Windows implementation of AsyncQueueManager class
 */

#include "../PCH.hpp"
#include "../AsyncQueueManager.hpp"
#include "../Logger.hpp"

#include <unistd.h>     /* for syscall() */
#include <sys/syscall.h>    /* for __NR_* definitions */
#include <algorithm>

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
    ::setupIo(mCtx);
    mQueueThread = std::thread(AsyncQueueManager::JobQueue);
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

bool AsyncQueueManager::EnqueueJob(JobProcedure callback, int eventFD, void* data)
{
    UNUSED(data);

    if (!mFdMap.count(eventFD)) // count() can return either 1 or 0
        mDescriptors.push_back({eventFD, POLLIN, 0});

    mFdMap[eventFD] = callback;
    return true; 
}

bool AsyncQueueManager::DequeueJob(int eventFD, void* data)
{
    UNUSED(data);

    if (mFdMap.erase(eventFD)) // erase(key) can return either 1 or 0
    {
        for (int i = 0; i < mDescriptors.size(); i++)
            if (mDescriptors[i].fd == eventFD)
            {
                mDescriptors.erase(mDescriptors.begin() + i);
                break;
            }
    }
    return true;
}

void AsyncQueueManager::JobQueue()
{
    const int pollTimeout = 500;  // poll() timeout in milliseconds
    int waitingEvents = 0;
    AsyncQueueManager* instance = &AsyncQueueManager::GetInstance();

    while (!instance->mIsDestroyed)
    {
        for (auto &i : instance->mDescriptors)
            i.revents = 0;

        // Poll to check if there are any events we may be interested in
        waitingEvents = ::poll(instance->mDescriptors.data(), instance->mDescriptors.size(), pollTimeout);
        if (waitingEvents < 0) // Error
        {
            LOG_ERROR("poll() for AsyncQueueManager failed: %s", strerror(errno));
            break;
        }
        else if (waitingEvents == 0) // Timeout
            continue;

        for (const auto&i : instance->mDescriptors)
            if (i.revents & POLLIN)
            {
                JobProcedure jobFunc = instance->mFdMap[i.fd];
                jobFunc(waitingEvents, i.fd);
            }
    }
}

aio_context_t AsyncQueueManager::GetQueueContext() const
{
    return mCtx;
}

} // namespace Common
} // namespace NFE
