/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's command buffer
 */

// TODO:
// 1. Improve logging, but be careful - functions from this source file will be called thousands
//    times per frame. Too much messages could flood a logger output.

#include "PCH.hpp"
#include "CommandBuffer.hpp"
#include "RendererD3D12.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "RenderTarget.hpp"
#include "PipelineState.hpp"
#include "Sampler.hpp"
#include "Translations.hpp"
#include "ResourceBinding.hpp"
#include "nfCommon/Logger.hpp"
#include "nfCommon/Win/Common.hpp"  // required for ID3DUserDefinedAnnotation



namespace NFE {
namespace Renderer {

CommandBuffer::CommandBuffer()
    : mCurrRenderTarget(nullptr)
    , mBindingLayout(nullptr)
    , mCurrBindingLayout(nullptr)
    , mCurrShaderProgram(nullptr)
    , mShaderProgram(nullptr)
    , mCurrPipelineState(nullptr)
    , mPipelineState(nullptr)
    , mCurrPrimitiveType(PrimitiveType::Unknown)
    , mFrameCounter(0)
    , mFrameCount(3) // TODO this must be configurable
    , mFrameBufferIndex(0)
{
}

bool CommandBuffer::Init(ID3D12Device* device)
{
    HRESULT hr;

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
        mFenceValues[i] = 0;
    }


    // create fence for frames synchronization
    if (FAILED(D3D_CALL_CHECK(gDevice->mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                                            IID_PPV_ARGS(&mFence)))))
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

    if (!mRingBuffer.Init(1024 * 1024))
    {
        LOG_ERROR("Failed to initialize ring buffer");
        return false;
    }

    return true;
}

CommandBuffer::~CommandBuffer()
{
    ::CloseHandle(mFenceEvent);
}

void CommandBuffer::Reset()
{
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
    mCurrShaderProgram = nullptr;
    mShaderProgram = nullptr;
    mCurrPipelineState = nullptr;
    mPipelineState = nullptr;
    mCurrPrimitiveType = PrimitiveType::Unknown;

    for (int i = 0; i < NFE_RENDERER_MAX_DYNAMIC_BUFFERS; ++i)
        mBoundDynamicBuffers[i] = nullptr;
}

void CommandBuffer::SetViewport(float left, float width, float top, float height,
                                float minDepth, float maxDepth)
{
    D3D12_VIEWPORT viewport;
    viewport.TopLeftX = left;
    viewport.TopLeftY = top;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = minDepth;
    viewport.MaxDepth = maxDepth;
    mCommandList->RSSetViewports(1, &viewport);
}

void CommandBuffer::SetScissors(int left, int top, int right, int bottom)
{
    D3D12_RECT rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    mCommandList->RSSetScissorRects(1, &rect);
}

void* CommandBuffer::MapBuffer(IBuffer* buffer, MapType type)
{
    UNUSED(buffer);
    UNUSED(type);
    return nullptr;
}

void CommandBuffer::UnmapBuffer(IBuffer* buffer)
{
    UNUSED(buffer);
}

void CommandBuffer::SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets)
{
    UNUSED(offsets);

    const int maxVertexBuffers = 4;
    D3D12_VERTEX_BUFFER_VIEW views[maxVertexBuffers];

    // TODO handle dynamic vertex buffers via ring buffer
    for (int i = 0; i < num; ++i)
    {
        Buffer* buffer = dynamic_cast<Buffer*>(vertexBuffers[i]);
        views[i].BufferLocation = buffer->mResource->GetGPUVirtualAddress();
        views[i].SizeInBytes = static_cast<UINT>(buffer->mSize);
        views[i].StrideInBytes = strides[i];
    }

    mCommandList->IASetVertexBuffers(0, num, views);
}

void CommandBuffer::SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format)
{
    Buffer* buffer = dynamic_cast<Buffer*>(indexBuffer);

    // TODO handle dynamic index buffers via ring buffer
    D3D12_INDEX_BUFFER_VIEW view;
    view.BufferLocation = buffer->mResource->GetGPUVirtualAddress();
    view.SizeInBytes = static_cast<UINT>(buffer->mSize);
    switch (format)
    {
    case IndexBufferFormat::Uint16:
        view.Format = DXGI_FORMAT_R16_UINT;
        break;
    case IndexBufferFormat::Uint32:
        view.Format = DXGI_FORMAT_R32_UINT;
        break;
    };

    mCommandList->IASetIndexBuffer(&view);
}

void CommandBuffer::BindResources(size_t slot, IResourceBindingInstance* bindingSetInstance)
{
    ResourceBindingInstance* instance = dynamic_cast<ResourceBindingInstance*>(bindingSetInstance);
    if (!instance)
        return;

    if (mCurrBindingLayout != mBindingLayout)
    {
        mCommandList->SetGraphicsRootSignature(mBindingLayout->mRootSignature.get());
        mCurrBindingLayout = mBindingLayout;
    }

    NFE_ASSERT(slot < mCurrBindingLayout->mBindingSets.size(), "Binding set index out of bounds");

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_GPU_DESCRIPTOR_HANDLE ptr = allocator.GetGpuHandle();
    ptr.ptr += instance->mDescriptorHeapOffset * allocator.GetDescriptorSize();
    mCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(slot), ptr);
}

void CommandBuffer::BindDynamicBuffer(size_t slot, IBuffer* buffer)
{
    NFE_ASSERT(slot < NFE_RENDERER_MAX_DYNAMIC_BUFFERS, "Invalid slot number");

    if (mCurrBindingLayout != mBindingLayout)
    {
        mCommandList->SetGraphicsRootSignature(mBindingLayout->mRootSignature.get());
        mCurrBindingLayout = mBindingLayout;
    }

    mBoundDynamicBuffers[slot] = dynamic_cast<Buffer*>(buffer);
}

void CommandBuffer::SetRenderTarget(IRenderTarget* renderTarget)
{
    if (mCurrRenderTarget == renderTarget)
        return;

    UnsetRenderTarget();

    mCurrRenderTarget = dynamic_cast<RenderTarget*>(renderTarget);

    if (mCurrRenderTarget != nullptr)
    {
        HeapAllocator& allocator = gDevice->GetRtvHeapAllocator();
        HeapAllocator& dsvAllocator = gDevice->GetDsvHeapAllocator();

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[MAX_RENDER_TARGETS];
        D3D12_CPU_DESCRIPTOR_HANDLE dsv;
        bool setDsv = false;

        for (size_t i = 0; i < mCurrRenderTarget->mTextures.size(); ++i)
        {
            Texture* tex = mCurrRenderTarget->mTextures[i];
            int currBuffer = tex->mCurrentBuffer;

            rtvs[i] = allocator.GetCpuHandle();
            rtvs[i].ptr += mCurrRenderTarget->mRTVs[currBuffer][i] * allocator.GetDescriptorSize();

            if (tex->mResourceState != D3D12_RESOURCE_STATE_RENDER_TARGET)
            {
                D3D12_RESOURCE_BARRIER rb;
                ZeroMemory(&rb, sizeof(rb));
                rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                rb.Transition.pResource = tex->mBuffers[currBuffer].get();
                rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                rb.Transition.StateBefore = tex->mResourceState;
                rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                mCommandList->ResourceBarrier(1, &rb);

                tex->mResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
            }
        }

        if (mCurrRenderTarget->mDSV != -1)
        {
            Texture* tex = mCurrRenderTarget->mDepthTexture;

            // TODO sometimes we may need ony read access
            if (tex->mResourceState != D3D12_RESOURCE_STATE_DEPTH_WRITE)
            {
                D3D12_RESOURCE_BARRIER rb;
                ZeroMemory(&rb, sizeof(rb));
                rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                rb.Transition.pResource = tex->mBuffers[0].get();
                rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                rb.Transition.StateBefore = tex->mResourceState;
                rb.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
                mCommandList->ResourceBarrier(1, &rb);

                tex->mResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
            }

            dsv = dsvAllocator.GetCpuHandle();
            dsv.ptr += mCurrRenderTarget->mDSV * dsvAllocator.GetDescriptorSize();
            setDsv = true;
        }

        mCommandList->OMSetRenderTargets(static_cast<UINT>(mCurrRenderTarget->mTextures.size()),
                                         rtvs, FALSE, setDsv ? &dsv : nullptr);
    }
}

void CommandBuffer::UnsetRenderTarget()
{
    if (mCurrRenderTarget != nullptr)
    {
        for (size_t i = 0; i < mCurrRenderTarget->mTextures.size(); ++i)
        {
            Texture* tex = mCurrRenderTarget->mTextures[i];
            int currBuffer = tex->mCurrentBuffer;

            // make transition to target state
            D3D12_RESOURCE_STATES targetState = tex->mTargetResourceState;
            if (tex->mResourceState != targetState)
            {
                D3D12_RESOURCE_BARRIER rb;
                ZeroMemory(&rb, sizeof(rb));
                rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                rb.Transition.pResource = tex->mBuffers[currBuffer].get();
                rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                rb.Transition.StateBefore = tex->mResourceState;
                rb.Transition.StateAfter = targetState;
                mCommandList->ResourceBarrier(1, &rb);

                tex->mResourceState = targetState;
            }
        }

        // unset depth texture if used
        if (mCurrRenderTarget->mDepthTexture != nullptr)
        {
            Texture* tex = mCurrRenderTarget->mDepthTexture;
            D3D12_RESOURCE_STATES targetState = tex->mTargetResourceState;
            if (tex->mResourceState != targetState)
            {
                D3D12_RESOURCE_BARRIER rb;
                ZeroMemory(&rb, sizeof(rb));
                rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                rb.Transition.pResource = tex->mBuffers[0].get();
                rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                rb.Transition.StateBefore = tex->mResourceState;
                rb.Transition.StateAfter = targetState;
                mCommandList->ResourceBarrier(1, &rb);

                tex->mResourceState = targetState;
            }
        }
    }
}

void CommandBuffer::SetShaderProgram(IShaderProgram* shaderProgram)
{
    mShaderProgram = dynamic_cast<ShaderProgram*>(shaderProgram);
}

void CommandBuffer::SetResourceBindingLayout(IResourceBindingLayout* layout)
{
    mBindingLayout = dynamic_cast<ResourceBindingLayout*>(layout);
}

void CommandBuffer::SetPipelineState(IPipelineState* state)
{
    mPipelineState = dynamic_cast<PipelineState*>(state);
}

void CommandBuffer::SetStencilRef(unsigned char ref)
{
    mCommandList->OMSetStencilRef(ref);
}


bool CommandBuffer::WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data)
{
    Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer);
    if (!bufferPtr)
    {
        LOG_ERROR("Invalid buffer");
        return false;
    }

    if (bufferPtr->mType != BufferType::Constant)
    {
        if (size > bufferPtr->mSize)
        {
            LOG_ERROR("Trying to perform write bigger than buffer size.");
            return false;
        }

        // TODO dynamic vertex and index buffers should be also handled via ring buffer

        size_t alignedSize = (size + 255) & ~255;
        size_t ringBufferOffset = mRingBuffer.Allocate(alignedSize);
        NFE_ASSERT(ringBufferOffset != RingBuffer::INVALID_OFFSET, "Ring buffer allocation failed");

        char* cpuPtr = reinterpret_cast<char*>(mRingBuffer.GetCpuAddress());
        cpuPtr += ringBufferOffset;
        memcpy(cpuPtr, data, size);

        // TODO this might be not the most optimal way of updating constant buffers...
        D3D12_RESOURCE_BARRIER rb;
        ZeroMemory(&rb, sizeof(rb));
        rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        rb.Transition.pResource = bufferPtr->mResource.get();
        rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        if (bufferPtr->mType == BufferType::Index)
            rb.Transition.StateBefore = D3D12_RESOURCE_STATE_INDEX_BUFFER;
        else
            rb.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        rb.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        mCommandList->ResourceBarrier(1, &rb);

        mCommandList->CopyBufferRegion(bufferPtr->mResource.get(), static_cast<UINT64>(offset),
                                       mRingBuffer.GetD3DResource(), static_cast<UINT64>(ringBufferOffset),
                                       static_cast<UINT64>(size));

        rb.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        if (bufferPtr->mType == BufferType::Index)
            rb.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
        else
            rb.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        mCommandList->ResourceBarrier(1, &rb);
    }
    else
    {
        // TODO temporary - offset and size support must be added
        NFE_ASSERT(bufferPtr->mAccess == BufferAccess::CPU_Write, "Buffer is not CPU-writeable");
        NFE_ASSERT(offset == 0, "Offset not supported");
        NFE_ASSERT(size == bufferPtr->mSize, "Size must cover the whole buffer");

        for (uint32 i = 0; i < NFE_RENDERER_MAX_DYNAMIC_BUFFERS; ++i)
        {
            if (mBoundDynamicBuffers[i] == buffer)
            {
                size_t ringBufferOffset = mRingBuffer.Allocate(bufferPtr->mRealSize);
                NFE_ASSERT(ringBufferOffset != RingBuffer::INVALID_OFFSET, "Ring buffer allocation failed");

                char* cpuPtr = reinterpret_cast<char*>(mRingBuffer.GetCpuAddress());
                cpuPtr += ringBufferOffset;
                memcpy(cpuPtr, data, size);

                D3D12_GPU_VIRTUAL_ADDRESS gpuPtr = mRingBuffer.GetGpuAddress();
                gpuPtr += ringBufferOffset;

                UINT rootParamIndex = static_cast<UINT>(mBindingLayout->mBindingSets.size()) + i;
                mCommandList->SetGraphicsRootConstantBufferView(rootParamIndex, gpuPtr);
            }
        }
    }

    return true;
}

void CommandBuffer::CopyTexture(ITexture* src, ITexture* dest)
{
    UNUSED(src);
    UNUSED(dest);
}

void CommandBuffer::Clear(int flags, const float* color, float depthValue,
                          unsigned char stencilValue)
{
    if (mCurrRenderTarget == nullptr)
        return;

    if (flags & NFE_CLEAR_FLAG_TARGET)
    {
        HeapAllocator& allocator = gDevice->GetRtvHeapAllocator();

        for (size_t i = 0; i < mCurrRenderTarget->mTextures.size(); ++i)
        {
            Texture* tex = mCurrRenderTarget->mTextures[i];
            int currentBuffer = tex->mCurrentBuffer;

            D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
            handle.ptr += mCurrRenderTarget->mRTVs[currentBuffer][i] * allocator.GetDescriptorSize();
            mCommandList->ClearRenderTargetView(handle, color, 0, nullptr);
        }
    }

    if (flags & (NFE_CLEAR_FLAG_DEPTH | NFE_CLEAR_FLAG_STENCIL))
    {
        HeapAllocator& dsvAllocator = gDevice->GetDsvHeapAllocator();

        if (mCurrRenderTarget->mDSV == -1)
            return;

        D3D12_CPU_DESCRIPTOR_HANDLE handle = dsvAllocator.GetCpuHandle();
        handle.ptr += mCurrRenderTarget->mDSV * dsvAllocator.GetDescriptorSize();

        D3D12_CLEAR_FLAGS clearFlags = static_cast<D3D12_CLEAR_FLAGS>(0);
        if (flags & NFE_CLEAR_FLAG_DEPTH)
            clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
        if (flags & NFE_CLEAR_FLAG_STENCIL)
            clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

        mCommandList->ClearDepthStencilView(handle, clearFlags, depthValue, stencilValue, 0, NULL);
    }
}

void CommandBuffer::UpdateStates()
{
    if (mCurrPipelineState != mPipelineState || mCurrShaderProgram != mShaderProgram)
    {
        if (mBindingLayout == nullptr)
            mBindingLayout = mPipelineState->mBindingLayout;
        else if (mBindingLayout != mPipelineState->mBindingLayout)
            LOG_ERROR("Resource binding layout mismatch");

        // set pipeline state
        const FullPipelineStateParts parts(mPipelineState, mShaderProgram);
        ID3D12PipelineState* state = gDevice->GetFullPipelineState(parts);
        mCommandList->SetPipelineState(state);
        mCurrPipelineState = mPipelineState;
        mCurrShaderProgram = mShaderProgram;

        // set root signature
        if (mCurrBindingLayout != mBindingLayout)
        {
            mCommandList->SetGraphicsRootSignature(mBindingLayout->mRootSignature.get());
            mCurrBindingLayout = mBindingLayout;
        }

        // set primitive type
        if (mCurrPipelineState->mPrimitiveType != mCurrPrimitiveType)
        {
            mCurrPrimitiveType = mCurrPipelineState->mPrimitiveType;
            D3D12_PRIMITIVE_TOPOLOGY topology = TranslatePrimitiveType(mCurrPrimitiveType,
                                                                       mCurrPipelineState->mNumControlPoints);
            mCommandList->IASetPrimitiveTopology(topology);
        }
    }
}

void CommandBuffer::Draw(int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    UpdateStates();
    mCommandList->DrawInstanced(vertexNum, instancesNum, vertexOffset, instanceOffset);
}

void CommandBuffer::DrawIndexed(int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    UpdateStates();
    mCommandList->DrawIndexedInstanced(indexNum, instancesNum, indexOffset, vertexOffset,
                                       instanceOffset);
}

std::unique_ptr<ICommandList> CommandBuffer::Finish()
{
    UnsetRenderTarget();

    D3D_CALL_CHECK(mCommandList->Close());

    // TODO: use memory pool
    std::unique_ptr<CommandList> list(new (std::nothrow) CommandList);
    if (!list)
    {
        LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    list->commandBuffer = this;
    return list;
}

bool CommandBuffer::MoveToNextFrame(ID3D12CommandQueue* commandQueue)
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

void CommandBuffer::BeginDebugGroup(const char* text)
{
    UNUSED(text);
}

void CommandBuffer::EndDebugGroup()
{
}

void CommandBuffer::InsertDebugMarker(const char* text)
{
    UNUSED(text);
}

} // namespace Renderer
} // namespace NFE
