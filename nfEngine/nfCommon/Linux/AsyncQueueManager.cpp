/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Windows implementation of AsyncQueueManager class
 */

#include "../PCH.hpp"
#include "../AsyncQueueManager.hpp"
#include "../Logger.hpp"
#include "Common.hpp"

#include <assert.h>


namespace NFE {
namespace Common {

AsyncQueueManager::AsyncQueueManager()
    : mIsDestroyed(false)
{
  //  mQueueThread = ::CreateThread(0, 0, AsyncQueueManager::JobQueue, nullptr, 0, 0);
   // mQuitEvent = ::CreateEvent(nullptr, FALSE, FALSE, L"JobQueue QuitEvent");
}

AsyncQueueManager::~AsyncQueueManager()
{
    mIsDestroyed = true;
   // ::SetEvent(mQuitEvent);
    //::WaitForSingleObject(mQueueThread, INFINITE);
}

AsyncQueueManager& AsyncQueueManager::GetInstance()
{
    static AsyncQueueManager instance;
    return instance;
}

bool AsyncQueueManager::EnqueueJob(JobProcedure& callback, void* data)
{
    //if (0 == ::QueueUserAPC(callback, mQueueThread, reinterpret_cast<ULONG_PTR>(data)))
    //{
     //   LOG_ERROR("QueueUserAPC() failed for AsyncQueueManager: %s", GetLastErrorString().c_str());
      //  return false;
    //}

    return true;
}

void AsyncQueueManager::JobQueue()
{
    const int pollTimeout = 500;  // poll() timeout in milliseconds
    const struct timespec eventsTimeout = {0, 500000};
    io_event eventsBuffer[NUM_EVENTS];
    int waitingEvents = 0;
    int eventsReturned = 0;
    struct ::pollfd pollFD;
    AsyncQueueManager* const instance = &AsyncQueueManager::GetInstance();

    while (!instance->mIsDestroyed)
    {
        // Reset poll descriptor
        pollFD = {instance->mJobEventFD, POLLIN, 0};

        // Poll to check if there are any events we may be interested in
        waitingEvents = ::poll(&pollFD, 1, pollTimeout);
        if (waitingEvents < 0) // Error
        {
            LOG_ERROR("poll() for AsyncQueueManager failed: %s", strerror(errno));
            break;
        }
        else if (waitingEvents == 0) // Timeout
            continue;

        // If waitingEvents > 0, we should get completed events from completion queue
        if (instance->mContext) // TODO needed?
            eventsReturned = ::io_getevents(instance->mContext, 1, waitingEvents, eventsBuffer, &eventsTimeout);
        else // TODO needed?
            break;

        if (eventsReturned < 0)
        {
            if (instance->mContext) // TODO needed?
                LOG_ERROR("io_getevents() for AsyncQueueManager failed: %s", strerror(errno));
            break;
        }
        else if (eventsReturned == 0)
        {
            // Being here means that poll() says there is some event completed, but io_getevents does not return it
            LOG_WARNING("poll() and io_getevents() mismatch in AsyncQueueManager!");
            
            // We're gonna consume this ficle event with read, so it won't bother us anymore.
            u_int64_t readReturn = 0;
            ::read(instance->mJobEventFD, &readReturn, sizeof(readReturn));
            continue;
        }

        //instance->eventsCallback() ?


        // Process events received from io_getevents()
        size_t bufferLimit = eventsBuffer + (eventsReturned * sizeof(struct ::io_event));
        for (::io_event* eventPtr = eventsBuffer; eventPtr < bufferLimit;)
        {
            struct ::iocb* iocbPtr = std::reinterpret_cast<::iocb*>(eventPtr->obj);

            // If we successfully got pointer to the event, we need to check if it's a pointer we want
            if (iocbPtr)
                if (iocbPtr->aio_lio_opcode == IOCB_CMD_PWRITE || iocbPtr->aio_lio_opcode == IOCB_CMD_PREAD)
                    FileAsync::FinishedOperationsHandler(eventPtr->res, std::reinterpret_cast<void*>(eventPtr->data));

            eventPtr += sizeof(struct ::io_event);
        }
    }
}

} // namespace Common
} // namespace NFE
