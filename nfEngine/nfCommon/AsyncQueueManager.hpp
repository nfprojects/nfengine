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
#include <linux/aio_abi.h>  /* for AIO types and constants */
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
    AsyncQueueManager();
    ~AsyncQueueManager();

public:
    bool mIsDestroyed;

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
    aio_context_t mCtx;
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
     * @param data     Pointer to the user-defined data that will be passed to the callback function.
     *
     * @remarks Calling this method second time with the same @FD will cease watching given descriptor.
     */
    bool EnqueueJob(JobProcedure callback, int FD, void* data);
    bool DequeueJob(int FD, void* data);

    /**
     * Get queue thread to enqueue asynchronous jobs on.
     */
    //const std::thread& GetQueueThread() const;
    aio_context_t GetQueueContext() const;
#endif
};

} // namespace Common
} // namespace NFE
