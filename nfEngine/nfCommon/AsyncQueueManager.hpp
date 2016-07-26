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
#endif

#include <functional>

namespace NFE {
namespace Common {

class NFCOMMON_API AsyncQueueManager
{
private:
    AsyncQueueManager();
    ~AsyncQueueManager();
    AsyncQueueManager(const AsyncQueueManager&) = delete;
    AsyncQueueManager(AsyncQueueManager&&) = delete;

#if defined(WIN32)
    HANDLE mQueueThread;
    HANDLE mQuitEvent;

    static DWORD CALLBACK JobQueue(LPVOID param);

#elif defined(__LINUX__) | defined(__linux__)
    std::thread mQueueThread;
    int mJobEventFD;
    aio_context_t mContext;

    static void JobQueue();

#endif
    bool mIsDestroyed;
public:
    using JobProcedure = PAPCFUNC;

    /**
     * Enqueue asynchronous job.
     * The callback function will be called from an internal worker thread.
     *
     * @param callback Funtion to be called upon end of jobs execution
     * @param data     Pointer to the user-defined data that will be passed to the callback function.
     */
    bool AsyncQueueManager::EnqueueJob(JobProcedure& callback, void* data);

    /**
     * Get instance of AsyncQueueManager
     */
    static AsyncQueueManager& GetInstance();
};

} // namespace Common
} // namespace NFE
