/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Windows implementation of AsyncQueueManager class
 */

#include "../PCH.hpp"
#include "../AsyncQueueManager.hpp"
#include "../Logger.hpp"
#include "Common.hpp"


namespace NFE {
namespace Common {

AsyncQueueManager::AsyncQueueManager()
    : mIsDestroyed(false)
{
    mQueueThread = ::CreateThread(0, 0, AsyncQueueManager::JobQueue, nullptr, 0, 0);
    mQuitEvent = ::CreateEvent(nullptr, FALSE, FALSE, L"JobQueue QuitEvent");
}

AsyncQueueManager::~AsyncQueueManager()
{
    mIsDestroyed = true;
    ::SetEvent(mQuitEvent);
    ::WaitForSingleObject(mQueueThread, INFINITE);
}

AsyncQueueManager& AsyncQueueManager::GetInstance()
{
    static AsyncQueueManager instance;
    return instance;
}

bool AsyncQueueManager::EnqueueJob(JobProcedure& callback, void* data)
{
    if (0 == ::QueueUserAPC(callback, mQueueThread, reinterpret_cast<ULONG_PTR>(data)))
    {
        LOG_ERROR("QueueUserAPC() failed for AsyncQueueManager: %s", GetLastErrorString().c_str());
        return false;
    }

    return true;
}

DWORD AsyncQueueManager::JobQueue(LPVOID param)
{
    UNUSED(param);
    AsyncQueueManager* const instance = &AsyncQueueManager::GetInstance();

    // Drift in the abyss of idleness waiting for the moment to become useful
    while (!instance->mIsDestroyed)
        ::WaitForSingleObjectEx(instance->mQuitEvent, INFINITE, true);

    return 0;
}

} // namespace Common
} // namespace NFE
