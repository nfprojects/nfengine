#include "PCH.hpp"
#include "Fence.hpp"
#include "Device.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"


namespace NFE {
namespace Renderer {

Fence::Fence(const FenceFlags flags)
    : mFlags(flags)
    , mSyncTask(Common::InvalidTaskID)
    , mFinished(false)
    , mLock()
{
}

Fence::~Fence()
{
    if (mFlags & FenceFlag_CpuWaitable)
    {
        NFE_ASSERT(mFinished.load(), "Fence should be finished when destroying. Otherwise it may create a deadlock as the dependency won't be fulfilled");
    }
}

bool Fence::Init()
{
    if (mFlags & FenceFlag_CpuWaitable)
    {
        Common::TaskDesc desc;
        desc.debugName = "NFE::Renderer::Fence::mSyncTask";
        desc.priority = Common::ThreadPool::MaxPriority;
        desc.function = [this](const Common::TaskContext&) {
            vkWaitForFences(gDevice->GetDevice(), 1, &mSynchronizable.mFence, VK_TRUE, UINT64_MAX);

            {
                NFE_SCOPED_LOCK(mLock);
                mFinished.exchange(true);
            }
        };

        mSyncTask = Common::ThreadPool::GetInstance().CreateTask(desc);
        if (mSyncTask == Common::InvalidTaskID)
        {
            NFE_LOG_ERROR("Failed to create Fence dependency task");
            return false;
        }

        VkFenceCreateInfo info;
        VK_ZERO_MEMORY(info);
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkResult result = vkCreateFence(gDevice->GetDevice(), &info, nullptr, &mSynchronizable.mFence);
        CHECK_VKRESULT(result, "Failed to create Vulkan Fence");
    }
    else if (mFlags & FenceFlag_GpuWaitable)
    {
        VkSemaphoreCreateInfo info;
        VK_ZERO_MEMORY(info);
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkResult result = vkCreateSemaphore(gDevice->GetDevice(), &info, nullptr, &mSynchronizable.mSemaphore);
        CHECK_VKRESULT(result, "Failed to create Vulkan Semaphore");
    }
    else
    {
        NFE_LOG_ERROR("Invalid Fence flag provided - cannot initialize");
        return false;
    }

    return true;
}

bool Fence::IsFinished() const
{
    NFE_ASSERT(mFlags & FenceFlag_CpuWaitable, "Cannot check if GPU-waitable Fence is finished");
    return mFinished;
}

void Fence::Sync(Common::TaskBuilder& taskBuilder)
{
    NFE_ASSERT(mFlags & FenceFlag_CpuWaitable, "Cannot check if GPU-waitable Fence is finished");

    NFE_SCOPED_LOCK(mLock);

    if (!mFinished)
    {
        taskBuilder.CustomTask(mSyncTask);
    }
}

} // namespace Renderer
} // namespace NFE
