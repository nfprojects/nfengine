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
public:
    using JobProcedure = PAPCFUNC;

    /**
    * Change directory monitor callback.
    * The callback function will be called from an internal worker thread.
    *
    * @param callback New callback function.
    */
    bool AsyncQueueManager::EnqueueJob(JobProcedure& callback, void* data);

    /**
    * Change directory monitor callback.
    * The callback function will be called from an internal worker thread.
    *
    * @param callback New callback function.
    */
    static AsyncQueueManager& GetInstance();

private:
    AsyncQueueManager();
    ~AsyncQueueManager();
#if defined(WIN32)
    HANDLE mQueueThread;
    static DWORD CALLBACK JobQueue(LPVOID param);
    void CallbackProc(ULONG_PTR arg);
    HANDLE mQuitEvent;
#elif defined(__LINUX__) | defined(__linux__)
#endif
    bool mIsDestroyed;
};

} // namespace Common
} // namespace NFE
