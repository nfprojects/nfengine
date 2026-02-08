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
}

CommandQueue::~CommandQueue()
{
    mFinishThread.store(true);

    // wait for all commands to complete before destroying the queue
    Signal()->Wait();

    // wait for the thread to finish
    mThread.Wait();
}

bool CommandQueue::Init(CommandQueueType type, const char* debugName)
{
    NFE_ASSERT(!mQueue, "Command queue is already initialized");

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = TranslateCommandListType(type);

    HRESULT hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(mQueue.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create %s command queue", CommandQueueTypeToStr(type));
        return false;
    }

    if (debugName && !SetDebugName(mQueue.Get(), Common::StringView(debugName)))
    {
        NFE_LOG_WARNING("Failed to set debug name of a command queue");
    }

    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create fence for %s command queue '%s'", CommandQueueTypeToStr(type), debugName);
        return false;
    }

    mType = type;

    mThread.RunFunction([this]()
    {
        FenceThreadFunc();
    });
    mThread.SetName("CommandQueueThread");
    mThread.SetPriority(ThreadPriority::AboveNormal);

    return true;
}

void CommandQueue::Submit(const Common::ArrayView<ICommandList*> commandLists, const Common::ArrayView<IFence*> waitFences)
{
    NFE_ASSERT(mQueue, "Command queue is not initialized");

    uint64_t fenceValue = 0;
    {
        NFE_SCOPED_LOCK(mLock);

        fenceValue = mFenceValue;

        for (IFence* fence : waitFences)
        {
            Fence* fencePtr = static_cast<Fence*>(fence);
            NFE_ASSERT(fencePtr, "Invalid fence");
            NFE_ASSERT(fencePtr->GetFlags() & FenceFlag_GpuWaitable, "Fence is not GPU-waitable");

            mQueue->Wait(fencePtr->GetD3DFence(), fencePtr->GetValue());
        }

        gDevice->GetCommandListManager()->ExecuteCommandList(*this, commandLists);

        // assign fence value to command lists
        for (ICommandList* commandList : commandLists)
        {
            NFE_ASSERT(commandList, "Invalid command list ptr");
            CommandList* typedCommandList = static_cast<CommandList*>(commandList);

            InternalCommandListPtr internalCommandList = typedCommandList->internalCommandList.Lock();
            NFE_ASSERT(internalCommandList != nullptr);

            internalCommandList->AssignFenceValue(mFence.Get(), mFenceValue);

            typedCommandList->internalCommandList.Reset();
        }
    }

    gDevice->GetRingBuffer()->FinishFrame(mFenceValue);
}

FencePtr CommandQueue::Signal(const FenceFlags flags)
{
    NFE_ASSERT(mQueue, "Command queue is not initialized");

    NFE_SCOPED_LOCK(mLock);

    uint64_t fenceValueToSignal = mFenceValue++;

    if (FAILED(D3D_CALL_CHECK(mQueue->Signal(mFence.Get(), fenceValueToSignal))))
    {
        NFE_LOG_ERROR("Failed to enqueue fence value update");
        return nullptr;
    }

    FencePtr fence = MakeSharedPtr<Fence>(fenceValueToSignal, flags, mFence.Get());

    PendingData pendingData;
    pendingData.fenceValue = fenceValueToSignal;
    pendingData.fencePtr = fence;
    mPendingData.PushBack(pendingData);

    return fence;
}

void CommandQueue::OnFenceCompleted(uint64_t fenceValue)
{
    // tick command list manager and global ring buffer automatically when fence is completed on GPU
    gDevice->GetCommandListManager()->OnFenceValueCompleted(mFence.Get(), fenceValue);
    gDevice->GetRingBuffer()->OnFenceValueCompleted(fenceValue);

    {
        NFE_SCOPED_LOCK(mLock);

        // find all pending fences that have been completed
        for (uint32_t i = 0; i < mPendingData.Size(); )
        {
            PendingData& pendingData = mPendingData[i];
            if (pendingData.fenceValue <= fenceValue)
            {
                FencePtr fencePtr = pendingData.fencePtr.Lock();
                if (fencePtr)
                {
                    static_cast<Fence*>(fencePtr.Get())->OnFenceFinished();
                }

                mPendingData.Erase(mPendingData.Begin() + i);
            }
            else
            {
                ++i;
            }
        }
    }
}

void CommandQueue::FenceThreadFunc()
{
    NFE_ASSERT(mFence, "Command queue fence is not initialized");

    HANDLE fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == INVALID_HANDLE_VALUE)
    {
        NFE_LOG_ERROR("Failed to create event object");
        return;
    }

    uint64_t waitFenceValue = InitialFenceValue;

    NFE_ASSERT(mQueue, "Command queue is not initialized");
    while (true)
    {
        // setup wait event
        HRESULT hr = D3D_CALL_CHECK(mFence->SetEventOnCompletion(waitFenceValue, fenceEvent));
        if (FAILED(hr))
        {
            NFE_LOG_ERROR("FenceThreadFunc: Failed to setup wait event, error code: %u", hr);
            continue;
        }

        // wait for fence to be signaled
        const DWORD waitResult = ::WaitForSingleObject(fenceEvent, INFINITE);
        if (waitResult != WAIT_OBJECT_0)
        {
            NFE_LOG_ERROR("FenceThreadFunc: WaitForSingleObject failed, error code: %u", ::GetLastError());
            continue;
        }

        // notify about fence completion
        OnFenceCompleted(waitFenceValue);

        // move to next fence value
        waitFenceValue++;

        if (mFinishThread)
            break;
    }
}

} // namespace Renderer
} // namespace NFE
