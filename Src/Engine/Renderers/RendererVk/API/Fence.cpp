#include "PCH.hpp"
#include "Fence.hpp"
#include "Device.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"


namespace NFE {
namespace Renderer {


bool FenceData::Init()
{
    if (flags & FenceFlag_CpuWaitable)
    {
        // TODO this should be preallocated and taken from a pool. This way we will not allocate a new
        //      Fence each time we call CommandQueue::Signal() - instead we can just reset a Fence and
        //      reuse it.
        VkFenceCreateInfo info;
        VK_ZERO_MEMORY(info);
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkResult result = vkCreateFence(gDevice->GetDevice(), &info, nullptr, &synchronizable.fence);
        CHECK_VKRESULT(result, "Failed to create Vulkan Fence");
    }
    else if (flags & FenceFlag_GpuWaitable)
    {
        VkSemaphoreCreateInfo info;
        VK_ZERO_MEMORY(info);
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkResult result = vkCreateSemaphore(gDevice->GetDevice(), &info, nullptr, &synchronizable.semaphore);
        CHECK_VKRESULT(result, "Failed to create Vulkan Semaphore");
    }
    else
    {
        NFE_LOG_ERROR("Invalid Fence flag provided - cannot initialize");
        return false;
    }

    return false;
}

FenceData::~FenceData()
{
    // can check any of the Synchronizable union here, both are a Vulkan handle
    if (synchronizable.fence != VK_NULL_HANDLE)
    {
        if (flags & FenceFlag_CpuWaitable)
            vkDestroyFence(gDevice->GetDevice(), synchronizable.fence, nullptr);
        else if (flags & FenceFlag_GpuWaitable)
            vkDestroySemaphore(gDevice->GetDevice(), synchronizable.semaphore, nullptr);
    }
}


Fence::Fence(const FenceFlags flags)
    : mSyncTask(Common::InvalidTaskID)
    , mFinished(false)
    , mLock()
    , mData(Common::MakeSharedPtr<FenceData>(flags, [this]() { OnSignalled(); }))
{
    NFE_ASSERT(mData->flags & (FenceFlag_CpuWaitable | FenceFlag_GpuWaitable), "Invalid Fence flags provided");
    NFE_ASSERT(mData->flags != (FenceFlag_CpuWaitable | FenceFlag_GpuWaitable), "Fence should have only one type");
}

Fence::~Fence()
{
    if (mData->flags & FenceFlag_CpuWaitable)
    {
        NFE_ASSERT(mFinished.load(), "Fence should be finished when destroying. Otherwise it may create a deadlock as the dependency won't be fulfilled");
    }

    mData.Reset();
}

bool Fence::Init()
{
    mData->Init();

    if (mData->flags & FenceFlag_CpuWaitable)
    {
        Common::TaskDesc desc;
        desc.debugName = "NFE::Renderer::Fence::mSyncTask";
        desc.priority = Common::ThreadPool::MaxPriority;
        mSyncTask = Common::ThreadPool::GetInstance().CreateTask(desc);
        if (mSyncTask == Common::InvalidTaskID)
        {
            NFE_LOG_ERROR("Failed to create Fence dependency task");
            return false;
        }
    }

    gDevice->GetFenceSignaller().RegisterFence(mData);

    return true;
}

void Fence::OnSignalled()
{
    NFE_SCOPED_LOCK(mLock);

    const bool wasFinished = mFinished.exchange(true);
    NFE_ASSERT(!wasFinished, "Fence was already finished, this should happen only once");

    Common::ThreadPool::GetInstance().DispatchTask(mSyncTask);
}

bool Fence::IsFinished() const
{
    NFE_ASSERT(mData->flags & FenceFlag_CpuWaitable, "Cannot check if GPU-waitable Fence is finished");
    return mFinished;
}

void Fence::Sync(Common::TaskBuilder& taskBuilder)
{
    NFE_ASSERT(mData->flags & FenceFlag_CpuWaitable, "Cannot sync to GPU-waitable Fence");

    NFE_SCOPED_LOCK(mLock);

    if (!mFinished)
        taskBuilder.CustomTask(mSyncTask);
}

} // namespace Renderer
} // namespace NFE
