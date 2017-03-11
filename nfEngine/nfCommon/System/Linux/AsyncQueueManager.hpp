/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  AsyncQueueManager utility declarations.
 */

#pragma once
#include "../nfCommon.hpp"

#include <linux/aio_abi.h>
#include <poll.h>
#include <functional>
#include <thread>
#include <unordered_map>


namespace NFE {
namespace Common {

class NFCOMMON_API AsyncQueueManager final
{
private:
    using JobProcedure = std::function<void(int eventsNo, int eventFD)>;
    std::thread mQueueThread;
    int mQuitEvent;
    std::atomic_bool mIsDestroyed;
    std::atomic_bool mIsInitialized;
    std::mutex mInitLock;
    ::aio_context_t mCtx;
    std::vector<::pollfd> mDescriptors;
    std::unordered_map<int, JobProcedure> mFdMap;

    AsyncQueueManager();
    ~AsyncQueueManager();
    bool Init();
    static void JobQueue();

public:
    /**
     * Get instance of AsyncQueueManager
     */
    static AsyncQueueManager& GetInstance();

    NFE_MAKE_NONMOVEABLE(AsyncQueueManager)
    NFE_MAKE_NONCOPYABLE(AsyncQueueManager)

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
};

} // namespace Common
} // namespace NFE
