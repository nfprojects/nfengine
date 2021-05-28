#include "PCH.hpp"
#include "CommandQueue.hpp"
#include "Device.hpp"
#include "Fence.hpp"
#include "CommandList.hpp"

#include "Internal/Debugger.hpp"

#include "Engine/Common/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

CommandQueue::CommandQueue()
    : mQueue(VK_NULL_HANDLE)
    , mType(CommandQueueType::Invalid)
    , mIndex(UINT32_MAX)
{
}

CommandQueue::~CommandQueue()
{
    Release();
}

bool CommandQueue::Init(CommandQueueType type, const char* debugName)
{
    NFE_ASSERT(gDevice->GetQueueFamilyManager().IsInitialized(), "QueueManager is not initialized");

    if (!gDevice->GetQueueFamilyManager().GetQueue(type, mQueue, mIndex))
    {
        NFE_LOG_ERROR("Failed to get a %s queue", CommandQueueTypeToStr(type));
        return false;
    }

    mType = type;

    if (debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mQueue), VK_OBJECT_TYPE_QUEUE, debugName);

    gDevice->GetCommandBufferManager(type).RegisterQueue(mQueue);

    return true;
}

void CommandQueue::Release()
{
    if (mQueue != VK_NULL_HANDLE)
    {
        gDevice->GetCommandBufferManager(mType).UnregisterQueue(mQueue);
        gDevice->GetQueueFamilyManager().FreeQueue(mType, mIndex);
    }
}

CommandQueueType CommandQueue::GetType() const
{
    return mType;
}

void CommandQueue::Submit(const Common::ArrayView<ICommandList*> commandLists,
                          const Common::ArrayView<IFence*> waitFences)
{
    // separate CpuWaitable Fences from GpuWaitable fences
    Common::DynArray<VkFence> fences;
    Common::DynArray<VkSemaphore> semaphores;
    Common::DynArray<VkPipelineStageFlags> semaphoreWaitStages;
    VkResult result = VK_SUCCESS;

    // All these DynArrays will never be fully filled at the same time.
    // However, it's faster to preallocate this space with a bit of backup instead of
    // PushBack'ing unknown amount of times with multiple reallocs
    fences.Reserve(waitFences.Size());
    semaphores.Reserve(waitFences.Size());
    semaphoreWaitStages.Reserve(waitFences.Size());
    for (IFence* fi: waitFences)
    {
        Fence* f = dynamic_cast<Fence*>(fi);
        NFE_ASSERT(f != nullptr, "Invalid Fence provided in waitFences argument");

        if (f->GetFlags() & FenceFlag_CpuWaitable)
        {
            fences.PushBack(f->GetSynchronizable().fence);
        }
        else if (f->GetFlags() & FenceFlag_GpuWaitable)
        {
            semaphores.PushBack(f->GetSynchronizable().semaphore);
            // TODO Wait stages are assumed to be TOP_OF_PIPE for now to ensure proper synchronization.
            //      This could be made faster by providing proper stages.
            semaphoreWaitStages.PushBack(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        }
        else
        {
            NFE_ASSERT(0, "Invalid flags in Fence provided via waitFences");
        }
    }

    // prepare command buffers
    Common::DynArray<VkCommandBuffer> commandBuffers(commandLists.Size());
    for (uint32 i = 0; i < commandLists.Size(); ++i)
    {
        CommandList* cl = dynamic_cast<CommandList*>(commandLists[i]);
        NFE_ASSERT(cl != nullptr, "Invalid Command List pointer provided");
        commandBuffers[i] = cl->GetCommandBuffer();
    }

    // Wait on VkFences if there are any
    if (fences.Size() > 0)
    {
        result = vkWaitForFences(gDevice->GetDevice(), fences.Size(), fences.Data(), VK_TRUE, UINT64_MAX);
        NFE_ASSERT(result == VK_SUCCESS, "Failed to wait for Vulkan Fences before submit: %d (%s)", result, TranslateVkResultToString(result));
    }

    // Submit command lists and possible wait for semaphores
    VkSubmitInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = commandBuffers.Size();
    info.pCommandBuffers = commandBuffers.Data();
    info.waitSemaphoreCount = semaphores.Size();
    info.pWaitSemaphores = semaphores.Data();
    info.pWaitDstStageMask = semaphoreWaitStages.Data();
    result = vkQueueSubmit(mQueue, 1, &info, VK_NULL_HANDLE);
    NFE_ASSERT(result == VK_SUCCESS, "Failed to submit Command Lists: %d (%s)", result, TranslateVkResultToString(result));
}

FencePtr CommandQueue::Signal(const FenceFlags flags)
{
    FencePtr fence = gDevice->CreateFence(flags);
    NFE_ASSERT(fence.Get() != nullptr, "Failed to create a Fence object for signalling");
    Fence* f = dynamic_cast<Fence*>(fence.Get());

    VkResult result = VK_SUCCESS;

    // TODO handle mutliple Signal() calls:
    //  - Cache created objects until the next Submit
    //  - Submit them all in one batch before submitting next CommandLists
    if (flags & FenceFlag_CpuWaitable)
    {
        NFE_LOG_INFO("Fence ptr: %p", f);
        result = vkQueueSubmit(mQueue, 0, nullptr, f->GetSynchronizable().fence);
        if (result != VK_SUCCESS)
        {
            NFE_LOG_ERROR("Failed to submit CpuWaitable fence: %d (%s)", result, TranslateVkResultToString(result));
            return nullptr;
        }
    }
    else
    {
        VkSubmitInfo info;
        VK_ZERO_MEMORY(info);
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &f->GetSynchronizable().semaphore;
        result = vkQueueSubmit(mQueue, 1, &info, VK_NULL_HANDLE);
        if (result != VK_SUCCESS)
        {
            NFE_LOG_ERROR("Failed to submit GpuWaitable fence: %d (%s)", result, TranslateVkResultToString(result));
            return nullptr;
        }
    }

    return fence;
}

} // namespace Renderer
} // namespace NFE
