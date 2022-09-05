#include "PCH.hpp"
#include "CommandBufferManager.hpp"

#include "Debugger.hpp"

#include "Engine/Common/Utils/StringUtils.hpp"
#include "Engine/Common/System/ConditionVariable.hpp"


namespace NFE {
namespace Renderer {

void DefaultOnFree() {}


CommandBufferManager::CommandBufferManager()
    : mDevice(VK_NULL_HANDLE)
    , mCommandPool(VK_NULL_HANDLE)
    , mLock()
    , mFirstUsed(0)
    , mCurrentAvailable(0)
    , mCommandBuffers()
    , mCBData()
    , mRegisteredQueues()
    , mQueueGuardFences()
{
}

CommandBufferManager::~CommandBufferManager()
{
    Release();
}

bool CommandBufferManager::Init(VkDevice device, const QueueFamilyManager& qfm, CommandQueueType queueType)
{
    VkResult result = VK_SUCCESS;
    Common::String poolName("CommandBufferManager-Pool-QueueType");
    Common::String cbName("CommandBufferManager-CommandBuffer-QueueType");

    mDevice = device;
    mQueueType = queueType;

    uint32 familyIndex = qfm.GetQueueFamily(queueType).familyIndex;

    VkCommandPoolCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = familyIndex;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(mDevice, &info, nullptr, &mCommandPool);
    CHECK_VKRESULT(result, "Failed to create Command Pool");

    if (Debugger::Instance().IsDebugAnnotationActive())
    {
        poolName += Util::CommandQueueTypeToString(mQueueType);
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mCommandPool),
                                        VK_OBJECT_TYPE_COMMAND_POOL,
                                        poolName.Str());
    }

    VkCommandBufferAllocateInfo cbInfo;
    VK_ZERO_MEMORY(cbInfo);
    cbInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbInfo.commandPool = mCommandPool;
    cbInfo.commandBufferCount = VK_COMMAND_BUFFER_POOL_SIZE;
    cbInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vkAllocateCommandBuffers(mDevice, &cbInfo, mCommandBuffers.Data());
    CHECK_VKRESULT(result, "Failed to allocate Command Buffers");

    if (Debugger::Instance().IsDebugAnnotationActive())
    {
        for (uint32 i = 0; i < mCommandBuffers.Size(); ++i)
        {
            Common::String name = cbName + Util::CommandQueueTypeToString(mQueueType) + "-#" + Common::ToString(i);
            Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mCommandPool),
                                            VK_OBJECT_TYPE_COMMAND_POOL,
                                            cbName.Str());
        }
    }

    for (auto& cbData: mCBData)
        cbData.state = State::Available;

    mFirstUsed = 0;
    mCurrentAvailable = 0;

    return true;
}

VkCommandBuffer CommandBufferManager::Acquire()
{
    NFE_SCOPED_LOCK(mLock);

    while (mCBData[mCurrentAvailable].state != State::Available)
    {
        mCurrentAvailable++;

        if (mCurrentAvailable == VK_COMMAND_BUFFER_POOL_SIZE)
            mCurrentAvailable = 0;

        NFE_ASSERT(mCurrentAvailable != mFirstUsed, "CB Manager full");
    }

    VkCommandBuffer buffer = mCommandBuffers[mCurrentAvailable];
    mCBData[mCurrentAvailable].state = State::Acquired;
    mCurrentAvailable++;
    if (mCurrentAvailable == VK_COMMAND_BUFFER_POOL_SIZE)
        mCurrentAvailable = 0;

    return buffer;
}

void CommandBufferManager::Free(VkCommandBuffer buffer, OnFreeCallback onFreeCb)
{
    NFE_SCOPED_LOCK(mLock);

    uint32 i = 0;
    for (; i < VK_COMMAND_BUFFER_POOL_SIZE; ++i)
    {
        if (mCommandBuffers[i] == buffer)
        {
            switch (mCBData[i].state)
            {
            case State::Acquired:
                mCBData[i].state = State::AwaitingFinish;
                mCBData[i].onFree = onFreeCb;
                break;
            case State::AwaitingFree:
                onFreeCb();
                mCBData[i].state = State::Available;
                break;
            default:
                NFE_ASSERT(0, "Invalid state");
            }
        }
    }

    NFE_ASSERT(i != mCurrentAvailable, "Provided invalid command buffer to free");
}

void CommandBufferManager::FinishFrame()
{
    NFE_SCOPED_LOCK(mLock);

    // we might not have any queues to keep track of (ex. if app doesn't use Compute queue at all)
    if (mRegisteredQueues.Size() == 0)
        return;

    NFE_ASSERT(mRegisteredQueues.Size() == mQueueGuardFences.Size(), "Invalid sizes - there should be same amount of queues as fences");

    VkResult result = VK_SUCCESS;

    // TODO this part could be done on a separate thread to speed up:
    // * Submit an event to a separate thread that we finished a frame
    // * If another "finish frame" is in progress (ex. waits on fences) we would stop there to keep in sync
    for (uint32 i = 0; i < mRegisteredQueues.Size(); ++i)
    {
        result = vkQueueSubmit(mRegisteredQueues[i], 0, nullptr, mQueueGuardFences[i]);
        NFE_ASSERT(result == VK_SUCCESS, "Failed to submit fence signal operation: %d (%s)", result, TranslateVkResultToString(result));
    }

    result = vkWaitForFences(mDevice, mQueueGuardFences.Size(), mQueueGuardFences.Data(), VK_TRUE, UINT64_MAX);
    NFE_ASSERT(result == VK_SUCCESS, "Failed to wait for all queue guard fences: %d (%s)", result, TranslateVkResultToString(result));

    result = vkResetFences(mDevice, mQueueGuardFences.Size(), mQueueGuardFences.Data());
    NFE_ASSERT(result == VK_SUCCESS, "Failed to reset queue guard fences: %d (%s)", result, TranslateVkResultToString(result));

    for (uint32 i = mFirstUsed; i != mCurrentAvailable; ++i)
    {
        if (i == VK_COMMAND_BUFFER_POOL_SIZE)
        {
            i = 0;
            if (i == mCurrentAvailable)
                break;
        }

        switch (mCBData[i].state)
        {
        case State::Acquired:
            mCBData[i].state = State::AwaitingFree;
            break;
        case State::AwaitingFinish:
            mCBData[i].onFree();
            mCBData[i].state = State::Available;
            break;
        default:
            NFE_ASSERT(0, "Invalid state");
        }
    }

    mFirstUsed = mCurrentAvailable;
}

bool CommandBufferManager::RegisterQueue(VkQueue queue)
{
    NFE_SCOPED_LOCK(mLock);

    NFE_ASSERT(mRegisteredQueues.Find(queue) == mRegisteredQueues.End(), "Queue already registered");

    mRegisteredQueues.PushBack(queue);

    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo fenceInfo;
    VK_ZERO_MEMORY(fenceInfo);
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkResult result = vkCreateFence(mDevice, &fenceInfo, nullptr, &fence);
    CHECK_VKRESULT(result, "Failed to create fence guarding queue");

    if (Debugger::Instance().IsDebugAnnotationActive())
    {
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(fence),
                                        VK_OBJECT_TYPE_FENCE,
                                        "NFE::Renderer::CommandBufferManager::QueueGuardFence");
    }

    mQueueGuardFences.PushBack(fence);
    return true;
}

bool CommandBufferManager::UnregisterQueue(VkQueue queue)
{
    NFE_SCOPED_LOCK(mLock);

    uint32 i = 0;
    for (; i < mRegisteredQueues.Size(); ++i)
    {
        if (mRegisteredQueues[i] == queue)
            break;
    }

    if (i == mRegisteredQueues.Size())
    {
        NFE_LOG_ERROR("Queue not registered");
        return false;
    }

    mRegisteredQueues[i] = mRegisteredQueues.Back();
    mRegisteredQueues.PopBack();

    vkDestroyFence(mDevice, mQueueGuardFences[i], nullptr);
    mQueueGuardFences[i] = mQueueGuardFences.Back();
    mQueueGuardFences.PopBack();

    return true;
}

void CommandBufferManager::Release()
{
    NFE_ASSERT(mRegisteredQueues.Empty(), "There are still queues registered - free CommandQueue objects first");

    if (mCommandPool != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(mDevice, mCommandPool, VK_COMMAND_BUFFER_POOL_SIZE, mCommandBuffers.Data());
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
        mCommandPool = VK_NULL_HANDLE;
    }
}

} // namespace Renderer
} // namespace NFE
