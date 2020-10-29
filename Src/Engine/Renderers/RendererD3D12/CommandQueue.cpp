/**
 * @file
 * @author  Witek902
 * @brief   D3D12 implementation of renderer's command queue
 */

#include "PCH.hpp"
#include "CommandQueue.hpp"
#include "CommandList.hpp"
#include "CommandListManager.hpp"
#include "RingBuffer.hpp"
#include "Device.hpp"
#include "RendererD3D12.hpp"

#include "Engine/Common/Utils/ScopedLock.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;

CommandQueueFamily GetCommandQueueFamily(CommandQueueType type)
{
    switch (type)
    {
    case CommandQueueType::Graphics:   return CommandQueueFamily::GraphicsCompute;
    case CommandQueueType::Copy:       return CommandQueueFamily::Copy;
    default:                    NFE_FATAL("Invalid command queue type");
    }
    return CommandQueueFamily::Invalid;
}

D3D12_COMMAND_LIST_TYPE TranslateCommandListType(CommandQueueType type)
{
    switch (type)
    {
    case CommandQueueType::Graphics:    return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case CommandQueueType::Compute:     return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case CommandQueueType::Copy:        return D3D12_COMMAND_LIST_TYPE_COPY;
    default:                            NFE_FATAL("Invalid command queue type");
    }
    return D3D12_COMMAND_LIST_TYPE_DIRECT;
}

CommandQueue::CommandQueue()
    : mType(CommandQueueType::Invalid)
{
    mFenceData.Init();
}

CommandQueue::~CommandQueue()
{
    // wait for all commands to complete before destroying the queue
    Signal()->Wait();
}

bool CommandQueue::Init(CommandQueueType type)
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = TranslateCommandListType(type);

    HRESULT hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(mQueue.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create %s command queue", CommandQueueTypeToStr(type));
        return false;
    }

    mFenceData.SetCallback([this](uint64 completedFenceValue)
    {
        // tick command list manager and global ring buffer automatically when fence is completed on GPU
        gDevice->GetCommandListManager()->OnFenceValueCompleted(&mFenceData, completedFenceValue);
        gDevice->GetRingBuffer()->OnFenceValueCompleted(&mFenceData, completedFenceValue);
    });

    return true;
}

void CommandQueue::Submit(const Common::ArrayView<ICommandList*> commandLists, const Common::ArrayView<IFence*> waitFences)
{
    uint64 fenceValue = 0;
    {
        NFE_SCOPED_LOCK(mLock);

        for (IFence* fence : waitFences)
        {
            Fence* fencePtr = static_cast<Fence*>(fence);
            NFE_ASSERT(fencePtr, "Invalid fence");

            mQueue->Wait(fencePtr->GetD3DFence(), fencePtr->GetValue());
        }

        gDevice->GetCommandListManager()->ExecuteCommandList(*this, commandLists);

        fenceValue = mFenceData.Signal(mQueue.Get());

        // assign fenceValue to command lists
        for (ICommandList* commandList : commandLists)
        {
            NFE_ASSERT(commandList, "Invalid command list ptr");
            CommandList* typedCommandList = static_cast<CommandList*>(commandList);

            InternalCommandListPtr internalCommandList = typedCommandList->internalCommandList.Lock();
            NFE_ASSERT(internalCommandList != nullptr);

            internalCommandList->AssignFenceValue(&mFenceData, fenceValue);

            typedCommandList->internalCommandList.Reset();
        }
    }

    gDevice->GetRingBuffer()->FinishFrame(&mFenceData, fenceValue);
}

FencePtr CommandQueue::Signal()
{
    FencePtr fence;
    mFenceData.Signal(mQueue.Get(), &fence);
    return fence;
}


} // namespace Renderer
} // namespace NFE
