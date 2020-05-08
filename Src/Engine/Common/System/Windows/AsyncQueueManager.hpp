/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  AsyncQueueManager utility declarations.
 */

#pragma once
#include "../../nfCommon.hpp"
#include "../../System/Mutex.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <atomic>


namespace NFE {
namespace Common {

class NFCOMMON_API AsyncQueueManager final
{
private:
    using JobProcedure = PAPCFUNC;
    HANDLE mQueueThread;
    HANDLE mQuitEvent;
    std::atomic_bool mIsDestroyed;
    std::atomic_bool mIsInitialized;
    Mutex mInitLock;

    AsyncQueueManager();
    ~AsyncQueueManager();
    bool Init();
    static DWORD CALLBACK JobQueue(LPVOID param);

public:
    /**
     * Get instance of AsyncQueueManager
     */
    static AsyncQueueManager& GetInstance();

    NFE_MAKE_NONMOVEABLE(AsyncQueueManager)
    NFE_MAKE_NONCOPYABLE(AsyncQueueManager)

    /**
     * Enqueue asynchronous job.
     * The callback function will be called from an internal worker thread.
     *
     * @param callback Funtion to be called upon end of jobs execution
     * @param data     Pointer to the user-defined data that will be passed to the callback function.
     */
    bool EnqueueJob(JobProcedure callback, void* data);
};

} // namespace Common
} // namespace NFE
