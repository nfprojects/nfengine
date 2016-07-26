/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  AsyncQueueManager utility declarations.
 */

#pragma once
#include "nfCommon.hpp"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#elif defined(__LINUX__) | defined(__linux__)
#include <linux/aio_abi.h>
#include <poll.h>
#include <functional>
#include <thread>
#include <unordered_map>
#endif


namespace NFE {
namespace Common {

class NFCOMMON_API AsyncQueueManager final
{
private:
    std::atomic_bool mIsDestroyed;

    AsyncQueueManager();
    ~AsyncQueueManager();
    bool Init();

public:
    /**
     * Get instance of AsyncQueueManager
     */
    static AsyncQueueManager& GetInstance();

    NFE_MAKE_NONMOVEABLE(AsyncQueueManager)
    NFE_MAKE_NONCOPYABLE(AsyncQueueManager)

#if defined(WIN32)
private:
    using JobProcedure = PAPCFUNC;
    HANDLE mQueueThread;
    HANDLE mQuitEvent;

    static DWORD CALLBACK JobQueue(LPVOID param);

public:
    /**
     * Enqueue asynchronous job.
     * The callback function will be called from an internal worker thread.
     *
     * @param callback Funtion to be called upon end of jobs execution
     * @param data     Pointer to the user-defined data that will be passed to the callback function.
     */
    bool EnqueueJob(JobProcedure callback, void* data);

#elif defined(__LINUX__) | defined(__linux__)
private:
    using JobProcedure = std::function<void(int eventsNo, int eventFD)>;
    std::thread mQueueThread;
    int mQuitEvent;

    ::aio_context_t mCtx;
    std::vector<::pollfd> mDescriptors;
    std::unordered_map<int, JobProcedure> mFdMap;

    static void JobQueue();

public:
    /**
     * Enqueue FD to be watched for POLLIN event.
     * The callback function will be called from an internal worker thread, when FD produces event.
     *
     * @param callback Funtion to be called upon POLLIN event of @FD
     * @param FD       Descriptor to be watched
     *
     * @remarks Callback will be called with number of events found and event descriptor.
     */
    bool EnqueueJob(JobProcedure callback, int FD);

    /**
     * Dequeue FD to stop watching for related events.
     *
     * @param FD Descriptor to stop watching
     */
    bool DequeueJob(int FD);

    /**
     * Get queue context, to enqueue asynchronous jobs with.
     */
    ::aio_context_t GetQueueContext() const;
#endif
};

} // namespace Common
} // namespace NFE
