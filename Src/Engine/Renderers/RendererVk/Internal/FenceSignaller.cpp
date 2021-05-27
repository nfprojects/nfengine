#include "PCH.hpp"
#include "FenceSignaller.hpp"

#include "API/Device.hpp"
#include "API/Fence.hpp"

#include "Engine/Common/Containers/StaticArray.hpp"


namespace NFE {
namespace Renderer {

FenceSignaller::FenceSignaller()
    : mThread()
    , mLock()
    , mDone(false)
{
}

FenceSignaller::~FenceSignaller()
{
}

void FenceSignaller::ThreadMain()
{
    constexpr uint32 MAX_FENCES = 8;
    Common::StaticArray<VkFence, MAX_FENCES> fencesToWait;
    Common::StaticArray<uint32, MAX_FENCES> fenceIDs;
    Common::StaticArray<uint32, MAX_FENCES> signalledFences;

    while (!mDone)
    {
        VkResult result = VK_SUCCESS;
        fencesToWait.Clear();
        fenceIDs.Clear();
        signalledFences.Clear();

        {
            NFE_SCOPED_LOCK(mLock);

            // collect fences to wait on
            for (uint32 i = 0; i < mRegisteredFences.Size(); ++i)
            {
                if (fencesToWait.Size() == MAX_FENCES)
                {
                    NFE_LOG_WARNING("Fence Signaller's fencesToWait cache full - some Fences had to be skipped and wait for next run. "
                                    "This could cause slowdowns - consider increasing MAX_FENCES here.");
                    break;
                }

                fencesToWait.PushBack(mRegisteredFences[i]->synchronizable.fence);
                fenceIDs.PushBack(i);
            }
        }

        // wait until Fence lights up
        // TODO For now it is a little bit hacked. vkWaitForFences can only wait for - you guessed it - VkFences.
        // A better solution (but more complex - requires some multi-platform code) would be:
        //  - In Fence class extract a platform-specific Waitable/Synchronizable primitive
        //     -> refer to vkGetFenceWin32HandleKHR/vkGetFenceFdKHR
        //  - Create an extra Waitable to wait on in addition to our fences (LoopEvent/LoopHandle? LoopFD?)
        //  - Signal LoopEvent when we need to leave the loop or interrupt the main waiting process
        do
        {
            if (fencesToWait.Size() > 0)
            {
                result = vkWaitForFences(gDevice->GetDevice(), fencesToWait.Size(), fencesToWait.Data(), VK_FALSE, MS_TO_NS(1000));
                if (result != VK_SUCCESS && result != VK_TIMEOUT)
                {
                    NFE_LOG_ERROR("Error while waiting for Fences: %d (%s)", result, TranslateVkResultToString(result));
                    mDone = true;
                }
            }
            else
            {
                // TODO HACK - do this properly via extracting handle from VkFence and adding our own event/CV to the pool
                Common::Thread::SleepCurrentThread(1.0);
                result = VK_SUCCESS;
            }

            if (mDone)
                break;
        }
        while (result == VK_TIMEOUT);

        if (result == VK_TIMEOUT)
            break;

        // at this point we should have at least one fence signalled - find out which ones
        for (uint32 i = 0; i < fencesToWait.Size(); ++i)
        {
            result = vkGetFenceStatus(gDevice->GetDevice(), fencesToWait[i]);
            if (result == VK_SUCCESS)
                signalledFences.PushBack(i);
            else if (result != VK_NOT_READY)
                NFE_LOG_ERROR("Error while checking Fence status: %d (%s)", result, TranslateVkResultToString(result));
        }

        // inform Fence that it's signalled and unregister it
        for (uint32 i = 0; i < signalledFences.Size(); ++i)
        {
            uint32 fenceDataIdx = fenceIDs[signalledFences[i]];
            mRegisteredFences[fenceDataIdx]->onSignalled();
            UnregisterFence(mRegisteredFences[fenceDataIdx]);
        }
    }
}

bool FenceSignaller::Init()
{
    mThread.SetName("NFE::RendererVk::FenceSignaller");
    mThread.SetPriority(Common::ThreadPriority::AboveNormal);
    mThread.RunFunction([this]()
    {
        ThreadMain();
    });

    return true;
}

void FenceSignaller::RegisterFence(const FenceDataPtr& fence)
{
    NFE_SCOPED_LOCK(mLock);

    NFE_ASSERT(mRegisteredFences.Find(fence) == mRegisteredFences.End(), "Fence already registered");
    mRegisteredFences.PushBack(fence);
}

void FenceSignaller::UnregisterFence(const FenceDataPtr& fence)
{
    NFE_SCOPED_LOCK(mLock);

    NFE_LOG_DEBUG("Unregister %p", fence.Get());
    Common::ArrayIterator<FenceDataPtr> it = mRegisteredFences.Find(fence);
    NFE_ASSERT(it != mRegisteredFences.End(), "Could not find fence to unregister");
    mRegisteredFences.Erase(it);

    // FIXME there is a need here to ping mThread about changes in registered fences, so the list
    //       of monitored fences is refreshed and we avoid use-after-free scenario. However,
    //       this requires reworking waiting mechanism a bit - see TODO above for details.
}

void FenceSignaller::Release()
{
    mDone = true;
    mThread.Wait();
    mRegisteredFences.Clear();
}


} // namespace Renderer
} // namespace NFE
