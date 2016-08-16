/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Windows implementation of AsyncQueueManager class
 */

#include "../PCH.hpp"
#include "AsyncQueueManager.hpp"
#include "../Logger.hpp"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>
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

} // namespace


namespace NFE {
namespace Common {

AsyncQueueManager::AsyncQueueManager()
    : mQuitEvent(0)
    , mIsDestroyed(false)
    , mIsInitialized(false)
    , mCtx(0)
{
}

AsyncQueueManager::~AsyncQueueManager()
{
    // Set destroy flag and shoot event, then wait for queue thread to decay
    if (mQueueThread.joinable())
    {
        mIsDestroyed = true;
        u_int64_t data = 1;
        ::write(mQuitEvent, &data, sizeof(data));
        mQueueThread.join();
    }

    // Clear events map
    mFdMap.clear();
    mDescriptors.clear();

    // Close context
    if (mCtx)
        ::io_destroy(mCtx);

    if (mQuitEvent)
        ::close(mQuitEvent);
}

AsyncQueueManager& AsyncQueueManager::GetInstance()
{
    static AsyncQueueManager instance;
    if (!instance.mIsInitialized)
    {
        std::lock_guard<std::mutex> lock(instance.mInitLock);
        if (!instance.mIsInitialized)
            instance.mIsInitialized = instance.Init();
    }

    return instance;
}

bool AsyncQueueManager::Init()
{
    if (::io_setup(NUM_EVENTS, &mCtx) < 0)
    {
        LOG_ERROR("io_setup() failed for AsyncQueueManager[%u]: %s", errno, strerror(errno));
        return false;
    }

    mQuitEvent = ::eventfd(0, EFD_NONBLOCK);
    if (mQuitEvent == -1)
    {
        LOG_ERROR("eventfd() failed for AsyncQueueManager[%u]: %s", errno, strerror(errno));
        return false;
    }

    mDescriptors.push_back({mQuitEvent, POLLIN, 0});

    mQueueThread = std::thread(AsyncQueueManager::JobQueue);
    if (!mQueueThread.joinable())
    {
        LOG_ERROR("std::thread() failed for AsyncQueueManager");
        return false;
    }

    return true;
}

bool AsyncQueueManager::EnqueueJob(JobProcedure callback, int eventFD)
{
    if (!mIsInitialized)
    {
        LOG_ERROR("AsyncQueueManager is not initialized!");
        return false;
    }

    if (!mFdMap.count(eventFD)) // count() can return either 1 or 0
        mDescriptors.push_back({eventFD, POLLIN, 0});

    mFdMap[eventFD] = callback;

    // We shoot mQuitEvent in order to poll for new jobs as well
    u_int64_t data = 0xFF;
    ::write(mQuitEvent, &data, sizeof(data));

    return true;
}

bool AsyncQueueManager::DequeueJob(int eventFD)
{
    if (!mIsInitialized)
    {
        LOG_ERROR("AsyncQueueManager is not initialized!");
        return false;
    }

    if (mFdMap.erase(eventFD)) // erase(key) can return either 1 or 0
    {
        for (int i = 0; i < mDescriptors.size(); i++)
            if (mDescriptors[i].fd == eventFD)
            {
                mDescriptors.erase(mDescriptors.begin() + i);
                break;
            }
    }

    // We shoot mQuitEvent in order to stop polling for deleted jobs
    u_int64_t data = 0xFF;
    ::write(mQuitEvent, &data, sizeof(data));



    return true;
}

void AsyncQueueManager::JobQueue()
{
    const int pollTimeout = 100;
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
                if (i.fd == instance->mQuitEvent)
                {
                    u_int64_t eval;
                    ::read(i.fd, &eval, sizeof(eval));
                    break;
                }

                JobProcedure jobFunc = instance->mFdMap[i.fd];
                if (jobFunc)
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
