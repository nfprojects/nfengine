/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of CommandListManager
 */

#include "PCH.hpp"
#include "CommandListManager.hpp"
#include "RendererD3D12.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

CommandListManager::CommandListManager()
    : mFrameCounter(0)
    , mFrameCount(3)
    , mFrameBufferIndex(0)
{
}

bool CommandListManager::Init(ID3D12Device* device)
{
    HRESULT hr;

    mFrameCounter = 1;
    mFenceValues.resize(mFrameCount);

    for (uint32 i = 0; i < mFrameCount; ++i)
    {
        D3DPtr<ID3D12CommandAllocator> commandAllocator;
        hr = D3D_CALL_CHECK(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                           IID_PPV_ARGS(&commandAllocator)));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 command allocator for frame %u (out of %u)", i, mFrameCount);
            return false;
        }

        mCommandAllocators.emplace_back(std::move(commandAllocator));
        mFenceValues[i] = 1;
    }


    // create fence for frames synchronization
    if (FAILED(D3D_CALL_CHECK(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)))))
    {
        LOG_ERROR("Failed to create D3D12 fence object");
        return false;
    }

    // create an event handle to use for frame synchronization
    mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (mFenceEvent == nullptr)
    {
        LOG_ERROR("Failed to create fence event object");
        return false;
    }


    // create D3D command list
    hr = D3D_CALL_CHECK(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                  mCommandAllocators[mFrameBufferIndex].get(), nullptr,
                                                  IID_PPV_ARGS(&mCommandList)));
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create D3D12 command list");
        return false;
    }

    // we don't want the command list to be in recording state
    hr = D3D_CALL_CHECK(mCommandList->Close());
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to close command list");
        return false;
    }

    return true;
}

CommandListManager::~CommandListManager()
{
    ::CloseHandle(mFenceEvent);
}

/*

void CommandListManager::Reset()
{
    if (mReset)
    {
        LOG_WARNING("Redundant command buffer reset");
        return;
    }

    HRESULT hr;

    hr = D3D_CALL_CHECK(mCommandAllocators[mFrameBufferIndex]->Reset());
    if (FAILED(hr))
        return;

    hr = D3D_CALL_CHECK(mCommandList->Reset(mCommandAllocators[mFrameBufferIndex].get(), nullptr));
    if (FAILED(hr))
        return;

    ID3D12DescriptorHeap* heaps[] =
    {
        gDevice->GetCbvSrvUavHeapAllocator().GetHeap(),
    };
    mCommandList->SetDescriptorHeaps(1, heaps);

    mCurrRenderTarget = nullptr;
    mBindingLayout = nullptr;
    mCurrBindingLayout = nullptr;
    mCurrPipelineState = nullptr;
    mPipelineState = nullptr;
    mCurrPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

    for (int i = 0; i < NFE_RENDERER_MAX_VOLATILE_CBUFFERS; ++i)
        mBoundVolatileCBuffers[i] = nullptr;

    mNumBoundVertexBuffers = 0;
    for (int i = 0; i < NFE_RENDERER_MAX_VERTEX_BUFFERS; ++i)
        mBoundVertexBuffers[i] = nullptr;

    mReset = true;
}

bool CommandListManager::MoveToNextFrame(ID3D12CommandQueue* commandQueue)
{
    uint64 currFenceValue = mFenceValues[mFrameBufferIndex];
    mRingBuffer.FinishFrame(currFenceValue);

    HRESULT hr = D3D_CALL_CHECK(commandQueue->Signal(mFence.get(), currFenceValue));
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to enqueue fence value update");
        return false;
    }

    // update frame index
    mFrameBufferIndex++;
    if (mFrameBufferIndex >= mFrameCount)
        mFrameBufferIndex = 0;

    // wait for frame
    UINT64 completedValue = mFence->GetCompletedValue();
    if (completedValue < mFenceValues[mFrameBufferIndex])
    {
        // TODO
        // Count how many times we enter this scope per second.
        // This means that we are render-bound.

        hr = D3D_CALL_CHECK(mFence->SetEventOnCompletion(mFenceValues[mFrameBufferIndex], mFenceEvent));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to set completion event for fence");
            return false;
        }

        if (WaitForSingleObject(mFenceEvent, INFINITE) != WAIT_OBJECT_0)
        {
            LOG_ERROR("WaitForSingleObject failed");
            return false;
        }
    }

    mRingBuffer.OnFrameCompleted(mFenceValues[mFrameBufferIndex]);

    mFenceValues[mFrameBufferIndex] = ++mFrameCounter;

    return true;
}

*/

CommandListPtr CommandListManager::ObtainCommandList()
{
    std::lock_guard<std::mutex> lock(mMutex);
}

bool CommandListManager::OnFinishCommandList(CommandList* commandList)
{
    std::lock_guard<std::mutex> lock(mMutex);
}

bool CommandListManager::OnDestroyCommandList(CommandList* commandList)
{
    std::lock_guard<std::mutex> lock(mMutex);
}

} // namespace Renderer
} // namespace NFE
