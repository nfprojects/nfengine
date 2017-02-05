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
#include "ComputePipelineState.hpp"
#include "Sampler.hpp"
#include "Translations.hpp"
#include "ResourceBinding.hpp"

#include "nfCommon/Assertion.hpp"
#include "nfCommon/Logger.hpp"
#include "nfCommon/Win/Common.hpp"  // required for ID3DUserDefinedAnnotation


namespace NFE {
namespace Renderer {

CommandBuffer::CommandBuffer()
    : mCurrRenderTarget(nullptr)
    , mBindingLayout(nullptr)
    , mCurrBindingLayout(nullptr)
    , mCurrPipelineState(nullptr)
    , mPipelineState(nullptr)
    , mComputeBindingLayout(nullptr)
    , mCurrComputePipelineState(nullptr)
    , mCurrPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
    , mFrameCounter(0)
    , mFrameCount(3) // TODO this must be configurable
    , mFrameBufferIndex(0)
    , mNumBoundVertexBuffers(0)
    , mReset(false)
{
    for (int i = 0; i < NFE_RENDERER_MAX_VOLATILE_CBUFFERS; ++i)
    {
        mBoundVolatileCBuffers[i] = nullptr;
        mBoundComputeVolatileCBuffers[i] = nullptr;
    }

    for (int i = 0; i < NFE_RENDERER_MAX_VERTEX_BUFFERS; ++i)
        mBoundVertexBuffers[i] = nullptr;
}

bool CommandBuffer::Init(ID3D12Device* device)
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

    // TODO: dynamic buffer growing
    if (!mRingBuffer.Init(8 * 1024 * 1024))
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
    mComputeBindingLayout = nullptr;
    mCurrComputePipelineState = nullptr;
    mCurrPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

    for (int i = 0; i < NFE_RENDERER_MAX_VOLATILE_CBUFFERS; ++i)
    {
        mBoundVolatileCBuffers[i] = nullptr;
        mBoundComputeVolatileCBuffers[i] = nullptr;
    }

    mNumBoundVertexBuffers = 0;
    for (int i = 0; i < NFE_RENDERER_MAX_VERTEX_BUFFERS; ++i)
        mBoundVertexBuffers[i] = nullptr;

    mReset = true;
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
    // TODO
    UNUSED(offsets);

    NFE_ASSERT(num < NFE_RENDERER_MAX_VERTEX_BUFFERS, "Too many vertex buffers");

    for (int i = 0; i < num; ++i)
    {
        D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = 0;
        uint32 size = 0;

        Buffer* buffer = dynamic_cast<Buffer*>(vertexBuffers[i]);
        if (!buffer)
        {
            LOG_ERROR("Invalid vertex buffer at slot %i", i);
            return;
        }

        if (buffer->GetResource())
        {
            gpuAddress = buffer->GetResource()->GetGPUVirtualAddress();
            size = buffer->GetSize();
        }

        mCurrVertexBufferViews[i].BufferLocation = gpuAddress;
        mCurrVertexBufferViews[i].SizeInBytes = size;
        mCurrVertexBufferViews[i].StrideInBytes = strides[i];

        mBoundVertexBuffers[i] = buffer;
    }

    mNumBoundVertexBuffers = num;
    mCommandList->IASetVertexBuffers(0, num, mCurrVertexBufferViews);
}

void CommandBuffer::SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format)
{
    Buffer* buffer = dynamic_cast<Buffer*>(indexBuffer);

    // TODO handle dynamic index buffers via ring buffer
    D3D12_INDEX_BUFFER_VIEW view;
    view.BufferLocation = buffer->GetResource()->GetGPUVirtualAddress();
    view.SizeInBytes = buffer->GetSize();
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

void CommandBuffer::BindVolatileCBuffer(size_t slot, IBuffer* buffer)
{
    NFE_ASSERT(slot < NFE_RENDERER_MAX_VOLATILE_CBUFFERS, "Invalid volatile buffer slot number");

    Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer);
    NFE_ASSERT(bufferPtr->GetMode() == BufferMode::Volatile, "Buffer mode must be volatile");

    if (mCurrBindingLayout != mBindingLayout)
    {
        mCommandList->SetGraphicsRootSignature(mBindingLayout->mRootSignature.get());
        mCurrBindingLayout = mBindingLayout;
    }

    mBoundVolatileCBuffers[slot] = bufferPtr;
}

void CommandBuffer::SetRenderTarget(IRenderTarget* renderTarget)
{
    if (mCurrRenderTarget == renderTarget)
        return;

    UnsetRenderTarget();

    mCurrRenderTarget = dynamic_cast<RenderTarget*>(renderTarget);

    D3D12_RESOURCE_BARRIER barriers[MAX_RENDER_TARGETS + 1];
    uint32 numBarriers = 0;

    D3D12_RESOURCE_BARRIER rb;
    ZeroMemory(&rb, sizeof(rb));
    rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

    if (mCurrRenderTarget != nullptr)
    {
        HeapAllocator& allocator = gDevice->GetRtvHeapAllocator();
        HeapAllocator& dsvAllocator = gDevice->GetDsvHeapAllocator();

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[MAX_RENDER_TARGETS];
        D3D12_CPU_DESCRIPTOR_HANDLE dsv;
        bool setDsv = false;

        size_t numTargets = mCurrRenderTarget->GetNumTargets();
        for (size_t i = 0; i < numTargets; ++i)
        {
            Texture* tex = mCurrRenderTarget->GetTexture(i);
            uint32 subResource = mCurrRenderTarget->GetSubresourceID(i);

            rtvs[i] = allocator.GetCpuHandle();
            rtvs[i].ptr += mCurrRenderTarget->GetRTV(i) * allocator.GetDescriptorSize();

            D3D12_RESOURCE_STATES targetState = D3D12_RESOURCE_STATE_RENDER_TARGET;
            if (tex->GetState(subResource) != targetState)
            {
                rb.Transition.pResource = tex->GetResource();
                rb.Transition.Subresource = subResource;
                rb.Transition.StateBefore = tex->GetState(subResource);
                rb.Transition.StateAfter = targetState;
                barriers[numBarriers++] = rb;
                tex->SetState(subResource, targetState);
            }
        }

        if (mCurrRenderTarget->GetDSV() != -1)
        {
            Texture* tex = mCurrRenderTarget->GetDepthTexture();
            uint32 subResource = mCurrRenderTarget->GetDepthTexSubresourceID();

            // TODO sometimes we may need ony read access
            D3D12_RESOURCE_STATES targetState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
            if (tex->GetState(subResource) != targetState)
            {
                rb.Transition.pResource = tex->GetResource();
                rb.Transition.Subresource = subResource;
                rb.Transition.StateBefore = tex->GetState(subResource);
                rb.Transition.StateAfter = targetState;
                barriers[numBarriers++] = rb;
                tex->SetState(subResource, targetState);
            }

            dsv = dsvAllocator.GetCpuHandle();
            dsv.ptr += mCurrRenderTarget->GetDSV() * dsvAllocator.GetDescriptorSize();
            setDsv = true;
        }

        if (numBarriers > 0)
            mCommandList->ResourceBarrier(numBarriers, barriers);

        mCommandList->OMSetRenderTargets(static_cast<UINT>(numTargets), rtvs, FALSE, setDsv ? &dsv : nullptr);
    }
}

void CommandBuffer::UnsetRenderTarget()
{
    // render targets + depth buffer
    D3D12_RESOURCE_BARRIER barriers[MAX_RENDER_TARGETS + 1];
    uint32 numBarriers = 0;

    D3D12_RESOURCE_BARRIER rb;
    ZeroMemory(&rb, sizeof(rb));
    rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

    if (mCurrRenderTarget != nullptr)
    {
        size_t numTargets = mCurrRenderTarget->GetNumTargets();
        for (size_t i = 0; i < numTargets; ++i)
        {
            Texture* tex = mCurrRenderTarget->GetTexture(i);
            uint32 subResource = mCurrRenderTarget->GetSubresourceID(i);
            D3D12_RESOURCE_STATES targetState = tex->GetTargetState();

            if (tex->GetState(subResource) != targetState)
            {
                rb.Transition.pResource = tex->GetResource();
                rb.Transition.Subresource = subResource;
                rb.Transition.StateBefore = tex->GetState(subResource);
                rb.Transition.StateAfter = targetState;
                barriers[numBarriers++] = rb;
                tex->SetState(subResource, targetState);
            }
        }

        // unset depth texture if used
        if (mCurrRenderTarget->GetDepthTexture() != nullptr)
        {
            Texture* tex = mCurrRenderTarget->GetDepthTexture();
            uint32 subResource = mCurrRenderTarget->GetDepthTexSubresourceID();
            D3D12_RESOURCE_STATES targetState = tex->GetTargetState();

            if (tex->GetState(subResource) != targetState)
            {
                rb.Transition.pResource = tex->GetResource();
                rb.Transition.Subresource = subResource;
                rb.Transition.StateBefore = tex->GetState(subResource);
                rb.Transition.StateAfter = targetState;
                barriers[numBarriers++] = rb;
                tex->SetState(subResource, targetState);
            }
        }
    }

    if (numBarriers)
        mCommandList->ResourceBarrier(numBarriers, barriers);
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

void CommandBuffer::WriteDynamicBuffer(Buffer* buffer, size_t offset, size_t size, const void* data)
{
    size_t alignedSize = (size + 255) & ~255;
    size_t ringBufferOffset = mRingBuffer.Allocate(alignedSize);
    NFE_ASSERT(ringBufferOffset != RingBuffer::INVALID_OFFSET, "Ring buffer allocation failed");

    // copy data from CPU memory to staging ring buffer
    char* cpuPtr = reinterpret_cast<char*>(mRingBuffer.GetCpuAddress());
    cpuPtr += ringBufferOffset;
    memcpy(cpuPtr, data, size);

    D3D12_RESOURCE_BARRIER rb;
    ZeroMemory(&rb, sizeof(rb));
    rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    rb.Transition.pResource = buffer->GetResource();
    rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // transit to copy-dest state
    rb.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
    rb.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    mCommandList->ResourceBarrier(1, &rb);

    // copy data from staging ring buffer to the target buffer
    mCommandList->CopyBufferRegion(buffer->GetResource(), static_cast<UINT64>(offset),
                                   mRingBuffer.GetD3DResource(), static_cast<UINT64>(ringBufferOffset),
                                   static_cast<UINT64>(size));

    // transit from copy-dest state
    rb.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    rb.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
    mCommandList->ResourceBarrier(1, &rb);
}

void CommandBuffer::WriteVolatileBuffer(Buffer* buffer, const void* data)
{
    size_t alignedSize = (buffer->GetSize() + 255) & ~255;
    size_t ringBufferOffset = mRingBuffer.Allocate(alignedSize);
    NFE_ASSERT(ringBufferOffset != RingBuffer::INVALID_OFFSET, "Ring buffer allocation failed");

    // copy data from CPU memory to ring buffer
    char* cpuPtr = reinterpret_cast<char*>(mRingBuffer.GetCpuAddress());
    cpuPtr += ringBufferOffset;
    memcpy(cpuPtr, data, buffer->GetSize());

    D3D12_GPU_VIRTUAL_ADDRESS gpuPtr = mRingBuffer.GetGpuAddress();
    gpuPtr += ringBufferOffset;

    // check if the buffer is not used as compute CBV
    if (mComputeBindingLayout)
    {
        for (uint32 i = 0; i < NFE_RENDERER_MAX_VOLATILE_CBUFFERS; ++i)
        {
            if (mBoundComputeVolatileCBuffers[i] == buffer)
            {
                UINT rootParamIndex = static_cast<UINT>(mComputeBindingLayout->mBindingSets.size()) + i;
                mCommandList->SetComputeRootConstantBufferView(rootParamIndex, gpuPtr);
            }
        }
    }

    // check if the buffer is not used as CBV
    for (uint32 i = 0; i < NFE_RENDERER_MAX_VOLATILE_CBUFFERS; ++i)
    {
        if (mBoundVolatileCBuffers[i] == buffer)
        {
            UINT rootParamIndex = static_cast<UINT>(mBindingLayout->mBindingSets.size()) + i;
            mCommandList->SetGraphicsRootConstantBufferView(rootParamIndex, gpuPtr);
        }
    }

    // check if the buffer is not used as VBV
    bool vbvUpdated = false;
    for (uint32 i = 0; i < mNumBoundVertexBuffers; ++i)
    {
        if (mBoundVertexBuffers[i] == buffer)
        {
            mCurrVertexBufferViews[i].BufferLocation = gpuPtr;
            mCurrVertexBufferViews[i].SizeInBytes = buffer->GetSize();
            vbvUpdated = true;
        }
    }

    if (vbvUpdated)
    {
        mCommandList->IASetVertexBuffers(0, mNumBoundVertexBuffers, mCurrVertexBufferViews);
    }
}

bool CommandBuffer::WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data)
{
    Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer);
    if (!bufferPtr)
    {
        LOG_ERROR("Invalid buffer");
        return false;
    }

    if (bufferPtr->GetMode() == BufferMode::Dynamic)
    {
        if (size > bufferPtr->GetSize())
        {
            LOG_ERROR("Trying to perform write bigger than buffer size.");
            return false;
        }

        WriteDynamicBuffer(bufferPtr, offset, size, data);
    }
    else if (bufferPtr->GetMode() == BufferMode::Volatile)
    {
        NFE_ASSERT(offset == 0, "Offset not supported");
        NFE_ASSERT(size == bufferPtr->GetSize(), "Size must cover the whole buffer");

        WriteVolatileBuffer(bufferPtr, data);
    }
    else
    {
        LOG_ERROR("Specified buffer can not be CPU-written");
        return false;
    }

    return true;
}

void CommandBuffer::CopyTexture(ITexture* src, ITexture* dest)
{
    Texture* srcTex = dynamic_cast<Texture*>(src);
    if (srcTex == nullptr)
    {
        LOG_ERROR("Invalid 'src' pointer");
        return;
    }

    if (srcTex->GetMode() == BufferMode::Readback || srcTex->GetMode() == BufferMode::Volatile)
    {
        LOG_ERROR("Can't copy from this texture");
        return;
    }

    Texture* destTex = dynamic_cast<Texture*>(dest);
    if (destTex == nullptr)
    {
        LOG_ERROR("Invalid 'dest' pointer");
        return;
    }

    if (destTex->GetMode() == BufferMode::Static || destTex->GetMode() == BufferMode::Volatile)
    {
        LOG_ERROR("Can't copy to this texture");
        return;
    }

    const D3D12_RESOURCE_STATES prevSrcTextureState = srcTex->GetState(0);
    const D3D12_RESOURCE_STATES prevDestTextureState = destTex->GetState(0);

    D3D12_RESOURCE_BARRIER rb;
    ZeroMemory(&rb, sizeof(rb));
    rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // transit source texture to "copy source" state
    if (prevSrcTextureState != D3D12_RESOURCE_STATE_COPY_SOURCE)
    {
        rb.Transition.pResource = srcTex->GetResource();
        rb.Transition.StateBefore = prevSrcTextureState;
        rb.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        mCommandList->ResourceBarrier(1, &rb);

        srcTex->SetState(0, D3D12_RESOURCE_STATE_COPY_SOURCE);
    }

    // transit destination texture to "copy destination" state
    if (prevDestTextureState != D3D12_RESOURCE_STATE_COPY_DEST)
    {
        rb.Transition.pResource = destTex->GetResource();
        rb.Transition.StateBefore = prevDestTextureState;
        rb.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        mCommandList->ResourceBarrier(1, &rb);

        destTex->SetState(0, D3D12_RESOURCE_STATE_COPY_DEST);
    }

    // perform copy
    if (destTex->GetMode() == BufferMode::Readback)
    {
        D3D12_TEXTURE_COPY_LOCATION sourceLoc;
        sourceLoc.pResource = srcTex->GetResource();
        sourceLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        sourceLoc.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION destLoc;
        destLoc.pResource = destTex->GetResource();
        destLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

        // get texture memory footprint information
        D3D12_RESOURCE_DESC srcResourceDesc = srcTex->GetResource()->GetDesc();
        gDevice->GetDevice()->GetCopyableFootprints(&srcResourceDesc, 0, 1, 0, &destLoc.PlacedFootprint, nullptr, nullptr, nullptr);

        // copy texture to buffer
        mCommandList->CopyTextureRegion(&destLoc, 0, 0, 0, &sourceLoc, nullptr);
    }
    else
    {
        // copy texture to texture
        mCommandList->CopyResource(destTex->GetResource(), srcTex->GetResource());
    }

    // transit source texture to previous state
    if (prevSrcTextureState != D3D12_RESOURCE_STATE_COPY_SOURCE)
    {
        rb.Transition.pResource = srcTex->GetResource();
        rb.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
        rb.Transition.StateAfter = prevSrcTextureState;
        mCommandList->ResourceBarrier(1, &rb);

        srcTex->SetState(0, prevSrcTextureState);
    }

    // transit destination texture to previous state
    if (prevDestTextureState != D3D12_RESOURCE_STATE_COPY_DEST)
    {
        rb.Transition.pResource = destTex->GetResource();
        rb.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        rb.Transition.StateAfter = prevDestTextureState;
        mCommandList->ResourceBarrier(1, &rb);

        destTex->SetState(0, prevDestTextureState);
    }
}

void CommandBuffer::Clear(int flags, uint32 numTargets, const uint32* slots,
                          const Math::Float4* colors, float depthValue, uint8 stencilValue)
{
    if (mCurrRenderTarget == nullptr)
        return;

    if (flags & ClearFlagsColor)
    {
        HeapAllocator& allocator = gDevice->GetRtvHeapAllocator();
        for (uint32 i = 0; i < numTargets; ++i)
        {
            uint32 slot = i;

            if (slots)
            {
                if (slots[i] >= mCurrRenderTarget->GetNumTargets())
                {
                    LOG_ERROR("Invalid render target texture slot = %u", slots[i]);
                    return;
                }

                slot = slots[i];
            }

            Texture* tex = mCurrRenderTarget->GetTexture(i);
            uint32 subResource = mCurrRenderTarget->GetSubresourceID(i);
            NFE_ASSERT(tex->GetState(subResource) == D3D12_RESOURCE_STATE_RENDER_TARGET, "Invalid resource state");

            D3D12_CPU_DESCRIPTOR_HANDLE handle = allocator.GetCpuHandle();
            handle.ptr += mCurrRenderTarget->GetRTV(slot) * allocator.GetDescriptorSize();
            mCommandList->ClearRenderTargetView(handle, reinterpret_cast<const float*>(&colors[i]), 0, nullptr);
        }
    }

    if (flags & (ClearFlagsDepth | ClearFlagsStencil))
    {
        HeapAllocator& dsvAllocator = gDevice->GetDsvHeapAllocator();

        if (mCurrRenderTarget->GetDSV() == -1)
            return;

        D3D12_CPU_DESCRIPTOR_HANDLE handle = dsvAllocator.GetCpuHandle();
        handle.ptr += mCurrRenderTarget->GetDSV() * dsvAllocator.GetDescriptorSize();

        D3D12_CLEAR_FLAGS clearFlags = static_cast<D3D12_CLEAR_FLAGS>(0);
        if (flags & ClearFlagsDepth)
            clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
        if (flags & ClearFlagsStencil)
            clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

        mCommandList->ClearDepthStencilView(handle, clearFlags, depthValue, stencilValue, 0, NULL);
    }
}

void CommandBuffer::UpdateStates()
{
    if (mCurrPipelineState != mPipelineState)
    {
        if (mBindingLayout == nullptr)
            mBindingLayout = mPipelineState->GetResBindingLayout();
        else if (mBindingLayout != mPipelineState->GetResBindingLayout())
            LOG_ERROR("Resource binding layout mismatch");

        // set pipeline state
        mCommandList->SetPipelineState(mPipelineState->GetPSO());
        mCurrPipelineState = mPipelineState;
        mCurrComputePipelineState = nullptr;

        // set root signature
        if (mCurrBindingLayout != mBindingLayout)
        {
            mCommandList->SetGraphicsRootSignature(mBindingLayout->mRootSignature.get());
            mCurrBindingLayout = mBindingLayout;
        }

        // set primitive type
        D3D_PRIMITIVE_TOPOLOGY newTopology = mPipelineState->GetPrimitiveTopology();
        if (newTopology != mCurrPrimitiveTopology)
        {
            mCurrPrimitiveTopology = newTopology;
            mCommandList->IASetPrimitiveTopology(newTopology);
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

void CommandBuffer::BindComputeResources(size_t slot, IResourceBindingInstance* bindingSetInstance)
{
    ResourceBindingInstance* instance = dynamic_cast<ResourceBindingInstance*>(bindingSetInstance);
    if (!instance)
    {
        // clear the slot
        D3D12_GPU_DESCRIPTOR_HANDLE ptr;
        ptr.ptr = 0;
        mCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(slot), ptr);
        return;
    }

    NFE_ASSERT(mComputeBindingLayout, "Compute binding layout not set");
    NFE_ASSERT(slot < mComputeBindingLayout->mBindingSets.size(), "Binding set index out of bounds");

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_GPU_DESCRIPTOR_HANDLE ptr = allocator.GetGpuHandle();
    ptr.ptr += instance->mDescriptorHeapOffset * allocator.GetDescriptorSize();
    mCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(slot), ptr);
}

void CommandBuffer::BindComputeVolatileCBuffer(size_t slot, IBuffer* buffer)
{
    NFE_ASSERT(slot < NFE_RENDERER_MAX_VOLATILE_CBUFFERS, "Invalid volatile buffer slot number");

    Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer);
    NFE_ASSERT(bufferPtr->GetMode() == BufferMode::Volatile, "Buffer mode must be volatile");

    mBoundComputeVolatileCBuffers[slot] = bufferPtr;
}

void CommandBuffer::SetComputeResourceBindingLayout(IResourceBindingLayout* layout)
{
    ResourceBindingLayout* newLayout = dynamic_cast<ResourceBindingLayout*>(layout);
    NFE_ASSERT(newLayout, "Invalid layout");

    if (mComputeBindingLayout != newLayout)
    {
        mCommandList->SetComputeRootSignature(newLayout->GetD3DRootSignature());
        mComputeBindingLayout = newLayout;
    }
}

void CommandBuffer::SetComputePipelineState(IComputePipelineState* state)
{
    ComputePipelineState* newState = dynamic_cast<ComputePipelineState*>(state);
    NFE_ASSERT(newState, "Invalid compute pipeline state");

    if (mCurrComputePipelineState != newState)
    {
        mCommandList->SetPipelineState(newState->GetPSO());
        mCurrComputePipelineState = newState;
    }

    mCurrPipelineState = nullptr;
}

void CommandBuffer::Dispatch(uint32 x, uint32 y, uint32 z)
{
    if (!mComputeBindingLayout)
    {
        LOG_ERROR("Binding layout not set");
        return;
    }

    if (!mCurrComputePipelineState)
    {
        LOG_ERROR("Compute pipeline state not set");
        return;
    }

    mCommandList->Dispatch(x, y, z);
}

std::unique_ptr<ICommandList> CommandBuffer::Finish()
{
    if (!mReset)
    {
        LOG_ERROR("Command buffer is not in recording state");
        return nullptr;
    }

    mReset = false;

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
