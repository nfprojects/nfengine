/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's command recorder
 */

// TODO:
// 1. Improve logging, but be careful - functions from this source file will be called thousands
//    times per frame. Too much messages could flood a logger output.

#include "PCH.hpp"
#include "CommandRecorder.hpp"
#include "CommandListManager.hpp"
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

#include "nfCommon/Math/Vector.hpp" // TODO remove
#include "nfCommon/System/Assertion.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/System/Win/Common.hpp"  // required for ID3DUserDefinedAnnotation
#include "nfCommon/Containers/StaticArray.hpp"


namespace NFE {
namespace Renderer {

CommandRecorder::CommandRecorder()
    : mCurrRenderTarget(nullptr)
    , mBindingLayout(nullptr)
    , mCurrBindingLayout(nullptr)
    , mCurrPipelineState(nullptr)
    , mPipelineState(nullptr)
    , mComputeBindingLayout(nullptr)
    , mCurrComputePipelineState(nullptr)
    , mCurrPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
    , mCommandList(nullptr)
    , mCommandListObject(nullptr)
    , mFrameCount(0)
    , mFrameCounter(1)
    , mFrameBufferIndex(0)
{
    for (int i = 0; i < NFE_RENDERER_MAX_VOLATILE_CBUFFERS; ++i)
    {
        mBoundVolatileCBuffers[i] = nullptr;
        mBoundComputeVolatileCBuffers[i] = nullptr;
    }

    for (int i = 0; i < NFE_RENDERER_MAX_VERTEX_BUFFERS; ++i)
        mBoundVertexBuffers[i] = nullptr;
}

bool CommandRecorder::Init(ID3D12Device* device, uint32 frameCount)
{
    HRESULT hr;

    mFrameCount = frameCount;
    mFrameCounter = 1;
    mFrameBufferIndex = 0;

    mReferencedResources.Resize(mFrameCount);
    mCommandAllocators.Resize(mFrameCount);
    for (uint32 i = 0; i < mFrameCount; ++i)
    {
        hr = D3D_CALL_CHECK(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mCommandAllocators[i].GetPtr())));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 command allocator for frame %u (out of %u)", i, mFrameCount);
            return false;
        }

        hr = D3D_CALL_CHECK(mCommandAllocators[i]->Reset());
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to reset command allocator for frame %u (out of %u)", i, mFrameCount);
            return false;
        }
    }

    if (!mRingBuffer.Init(1024 * 1024))
    {
        LOG_ERROR("Failed to initialize ring buffer");
        return false;
    }

    return true;
}

CommandRecorder::~CommandRecorder()
{
    NFE_ASSERT(mCommandList == nullptr, "Finish() not called before command buffer destruction");
}

bool CommandRecorder::Begin()
{
    if (mCommandList)
    {
        LOG_WARNING("Redundant command buffer reset");
        return false;
    }

    // get a free command list from the command list manager
    ID3D12CommandAllocator* currentCommandAllocator = mCommandAllocators[mFrameBufferIndex].Get();
    mCommandListObject = gDevice->GetCommandListManager()->RequestCommandList(currentCommandAllocator);
    if (!mCommandListObject)
    {
        LOG_ERROR("Failed to optain command list");
        return false;
    }

    mCommandList = mCommandListObject->GetD3DCommandList();

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

    return true;
}

CommandListID CommandRecorder::Finish()
{
    NFE_ASSERT(mCommandList, "Command buffer is not in recording state");

    Internal_UnsetRenderTarget();

    // verify resource states
    mResourceStateCache.OnFinishCommandBuffer();

    HRESULT hr = D3D_CALL_CHECK(mCommandList->Close());
    if (FAILED(hr))
    {
        // recording failed
        mCommandListObject = nullptr;
        mCommandList = nullptr;
        return INVALID_COMMAND_LIST_ID;
    }

    // TODO pass current frame ID to the command list
    const CommandListID id = gDevice->GetCommandListManager()->OnCommandListRecorded(mCommandListObject);
    mCommandListObject = nullptr;
    mCommandList = nullptr;
    return id;
}

bool CommandRecorder::OnFinishFrame(uint64 fenceValue)
{
    mRingBuffer.FinishFrame(fenceValue);
    return true;
}

bool CommandRecorder::OnFrameCompleted(uint64 fenceValue)
{
    mFrameBufferIndex++;
    if (mFrameBufferIndex >= mFrameCount)
        mFrameBufferIndex = 0;

    HRESULT hr = D3D_CALL_CHECK(mCommandAllocators[mFrameBufferIndex]->Reset());
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to reset command allocator for frame %u", mFrameBufferIndex);
        return false;
    }

    // drop resources references
    mReferencedResources[mFrameBufferIndex].Clear();

    mRingBuffer.OnFrameCompleted(fenceValue);
    return true;
}

void CommandRecorder::Internal_ReferenceResource(const ResourcePtr& resource)
{
    NFE_ASSERT(mCommandList, "Command buffer is not in recording state");

    ReferencedResources& resourcesList = mReferencedResources[mFrameBufferIndex];
    resourcesList.PushBack(resource);
}

//////////////////////////////////////////////////////////////////////////

void CommandRecorder::SetViewport(float left, float width, float top, float height, float minDepth, float maxDepth)
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

void CommandRecorder::SetScissors(int left, int top, int right, int bottom)
{
    D3D12_RECT rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    mCommandList->RSSetScissorRects(1, &rect);
}

void* CommandRecorder::MapBuffer(const BufferPtr& buffer, MapType type)
{
    UNUSED(buffer);
    UNUSED(type);
    return nullptr;
}

void CommandRecorder::UnmapBuffer(const BufferPtr& buffer)
{
    UNUSED(buffer);
}

void CommandRecorder::SetVertexBuffers(int num, const BufferPtr* vertexBuffers, int* strides, int* offsets)
{
    // TODO
    UNUSED(offsets);

    NFE_ASSERT(num < NFE_RENDERER_MAX_VERTEX_BUFFERS, "Too many vertex buffers");

    for (int i = 0; i < num; ++i)
    {
        D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = 0;
        uint32 size = 0;

        const Buffer* buffer = dynamic_cast<Buffer*>(vertexBuffers[i].Get());
        NFE_ASSERT(buffer, "Invalid vertex buffer at slot %i", i);

        if (buffer->GetResource())
        {
            Internal_ReferenceResource(Common::DynamicCast<Resource>(vertexBuffers[i]));
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

void CommandRecorder::SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format)
{
    Internal_ReferenceResource(indexBuffer);
    const Buffer* buffer = dynamic_cast<Buffer*>(indexBuffer.Get());

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

void CommandRecorder::BindResources(size_t slot, const ResourceBindingInstancePtr& bindingSetInstance)
{
    ResourceBindingInstance* instance = dynamic_cast<ResourceBindingInstance*>(bindingSetInstance.Get());
    if (!instance)
        return;

    Internal_ReferenceResource(bindingSetInstance);

    if (mCurrBindingLayout != mBindingLayout)
    {
        mCommandList->SetGraphicsRootSignature(mBindingLayout->mRootSignature.Get());
        mCurrBindingLayout = mBindingLayout;
    }

    NFE_ASSERT(slot < mCurrBindingLayout->mBindingSets.size(), "Binding set index out of bounds");

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_GPU_DESCRIPTOR_HANDLE ptr = allocator.GetGpuHandle();
    ptr.ptr += instance->mDescriptorHeapOffset * allocator.GetDescriptorSize();
    mCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(slot), ptr);
}

void CommandRecorder::BindVolatileCBuffer(size_t slot, const BufferPtr& buffer)
{
    NFE_ASSERT(slot < NFE_RENDERER_MAX_VOLATILE_CBUFFERS, "Invalid volatile buffer slot number");

    const Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(bufferPtr->GetMode() == BufferMode::Volatile, "Buffer mode must be volatile");

    if (mCurrBindingLayout != mBindingLayout)
    {
        mCommandList->SetGraphicsRootSignature(mBindingLayout->mRootSignature.Get());
        mCurrBindingLayout = mBindingLayout;
    }

    mBoundVolatileCBuffers[slot] = bufferPtr;
}

void CommandRecorder::SetRenderTarget(const RenderTargetPtr& renderTarget)
{
    if (mCurrRenderTarget == renderTarget.Get())
        return;

    Internal_UnsetRenderTarget();

    mCurrRenderTarget = dynamic_cast<RenderTarget*>(renderTarget.Get());

    Common::StaticArray<D3D12_RESOURCE_BARRIER, MAX_RENDER_TARGETS + 1> barriers;

    D3D12_RESOURCE_BARRIER rb;
    ZeroMemory(&rb, sizeof(rb));
    rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

    if (!mCurrRenderTarget)
        return;

    Internal_ReferenceResource(renderTarget);

    HeapAllocator& allocator = gDevice->GetRtvHeapAllocator();
    HeapAllocator& dsvAllocator = gDevice->GetDsvHeapAllocator();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[MAX_RENDER_TARGETS];
    D3D12_CPU_DESCRIPTOR_HANDLE dsv;
    bool setDsv = false;

    size_t numTargets = mCurrRenderTarget->GetNumTargets();
    for (size_t i = 0; i < numTargets; ++i)
    {
        const InternalTexturePtr& tex = mCurrRenderTarget->GetTexture(i);
        uint32 subResource = mCurrRenderTarget->GetSubresourceID(i);

        rtvs[i] = allocator.GetCpuHandle();
        rtvs[i].ptr += mCurrRenderTarget->GetRTV(i) * allocator.GetDescriptorSize();

        const D3D12_RESOURCE_STATES targetState = D3D12_RESOURCE_STATE_RENDER_TARGET;
        const D3D12_RESOURCE_STATES previousState = mResourceStateCache.SetResourceState(tex.Get(), subResource, targetState);
        if (previousState != targetState)
        {
            rb.Transition.pResource = tex->GetResource();
            rb.Transition.Subresource = subResource;
            rb.Transition.StateBefore = previousState;
            rb.Transition.StateAfter = targetState;

            barriers.PushBack(rb);
        }
    }

    if (mCurrRenderTarget->GetDSV() != -1)
    {
        const InternalTexturePtr& tex = mCurrRenderTarget->GetDepthTexture();
        uint32 subResource = mCurrRenderTarget->GetDepthTexSubresourceID();

        // TODO sometimes we may need only read access
        const D3D12_RESOURCE_STATES targetState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        const D3D12_RESOURCE_STATES previousState = mResourceStateCache.SetResourceState(tex.Get(), subResource, targetState);
        if (previousState != targetState)
        {
            rb.Transition.pResource = tex->GetResource();
            rb.Transition.Subresource = subResource;
            rb.Transition.StateBefore = previousState;
            rb.Transition.StateAfter = targetState;

            barriers.PushBack(rb);
        }

        dsv = dsvAllocator.GetCpuHandle();
        dsv.ptr += mCurrRenderTarget->GetDSV() * dsvAllocator.GetDescriptorSize();
        setDsv = true;
    }

    if (!barriers.Empty())
    {
        mCommandList->ResourceBarrier(barriers.Size(), barriers.Data());
    }

    mCommandList->OMSetRenderTargets(static_cast<UINT>(numTargets), rtvs, FALSE, setDsv ? &dsv : nullptr);
}

void CommandRecorder::Internal_UnsetRenderTarget()
{
    if (mCurrRenderTarget == nullptr)
        return;

    // render targets + depth buffer
    Common::StaticArray<D3D12_RESOURCE_BARRIER, MAX_RENDER_TARGETS + 1> barriers;

    D3D12_RESOURCE_BARRIER rb;
    ZeroMemory(&rb, sizeof(rb));
    rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

    size_t numTargets = mCurrRenderTarget->GetNumTargets();
    for (size_t i = 0; i < numTargets; ++i)
    {
        const InternalTexturePtr& tex = mCurrRenderTarget->GetTexture(i);
        uint32 subResource = mCurrRenderTarget->GetSubresourceID(i);

        const D3D12_RESOURCE_STATES targetState = tex->GetDefaultState();
        const D3D12_RESOURCE_STATES previousState = mResourceStateCache.SetResourceState(tex.Get(), subResource, targetState);

        if (previousState != targetState)
        {
            rb.Transition.pResource = tex->GetResource();
            rb.Transition.Subresource = subResource;
            rb.Transition.StateBefore = previousState;
            rb.Transition.StateAfter = targetState;

            barriers.PushBack(rb);
        }
    }

    // unset depth texture if used
    if (mCurrRenderTarget->GetDepthTexture() != nullptr)
    {
        const InternalTexturePtr& tex = mCurrRenderTarget->GetDepthTexture();
        uint32 subResource = mCurrRenderTarget->GetDepthTexSubresourceID();

        const D3D12_RESOURCE_STATES targetState = tex->GetDefaultState();
        const D3D12_RESOURCE_STATES previousState = mResourceStateCache.SetResourceState(tex.Get(), subResource, targetState);

        if (previousState != targetState)
        {
            rb.Transition.pResource = tex->GetResource();
            rb.Transition.Subresource = subResource;
            rb.Transition.StateBefore = previousState;
            rb.Transition.StateAfter = targetState;

            barriers.PushBack(rb);
        }
    }

    if (!barriers.Empty())
    {
        mCommandList->ResourceBarrier(barriers.Size(), barriers.Data());
    }
}

void CommandRecorder::SetResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    Internal_ReferenceResource(layout);
    mBindingLayout = dynamic_cast<ResourceBindingLayout*>(layout.Get());
}

void CommandRecorder::SetPipelineState(const PipelineStatePtr& state)
{
    Internal_ReferenceResource(state);
    mPipelineState = dynamic_cast<PipelineState*>(state.Get());
}

void CommandRecorder::SetStencilRef(unsigned char ref)
{
    mCommandList->OMSetStencilRef(ref);
}

void CommandRecorder::Internal_WriteDynamicBuffer(Buffer* buffer, size_t offset, size_t size, const void* data)
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

void CommandRecorder::Internal_WriteVolatileBuffer(Buffer* buffer, const void* data)
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

bool CommandRecorder::WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data)
{
    Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(bufferPtr, "Invalid buffer");

    Internal_ReferenceResource(buffer);

    if (bufferPtr->GetMode() == BufferMode::Dynamic)
    {
        if (size > bufferPtr->GetSize())
        {
            LOG_ERROR("Trying to perform write bigger than buffer size.");
            return false;
        }

        Internal_WriteDynamicBuffer(bufferPtr, offset, size, data);
    }
    else if (bufferPtr->GetMode() == BufferMode::Volatile)
    {
        NFE_ASSERT(offset == 0, "Offset not supported");
        NFE_ASSERT(size == bufferPtr->GetSize(), "Size must cover the whole buffer");

        Internal_WriteVolatileBuffer(bufferPtr, data);
    }
    else
    {
        LOG_ERROR("Specified buffer can not be CPU-written");
        return false;
    }

    return true;
}

void CommandRecorder::CopyTexture(const TexturePtr& src, const TexturePtr& dest)
{
    NFE_ASSERT(src, "Invalid source texture");
    NFE_ASSERT(dest, "Invalid destination texture");

    Internal_ReferenceResource(src);
    Internal_ReferenceResource(dest);

    Texture* srcTex = dynamic_cast<Texture*>(src.Get());
    NFE_ASSERT(src, "Invalid 'src' pointer");

    if (srcTex->GetMode() == BufferMode::Readback || srcTex->GetMode() == BufferMode::Volatile)
    {
        LOG_ERROR("Can't copy from this texture");
        return;
    }

    Texture* destTex = dynamic_cast<Texture*>(dest.Get());
    NFE_ASSERT(destTex, "Invalid 'dest' pointer");

    if (destTex->GetMode() == BufferMode::Static || destTex->GetMode() == BufferMode::Volatile)
    {
        LOG_ERROR("Can't copy to this texture");
        return;
    }

    Common::StaticArray<D3D12_RESOURCE_BARRIER, 2> barriers;

    D3D12_RESOURCE_BARRIER rb;
    ZeroMemory(&rb, sizeof(rb));
    rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // transit source texture to "copy source" state
    const D3D12_RESOURCE_STATES prevSrcTextureState = mResourceStateCache.SetResourceState(srcTex, 0, D3D12_RESOURCE_STATE_COPY_SOURCE);
    if (prevSrcTextureState != D3D12_RESOURCE_STATE_COPY_SOURCE)
    {
        rb.Transition.pResource = srcTex->GetResource();
        rb.Transition.StateBefore = prevSrcTextureState;
        rb.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        barriers.PushBack(rb);
    }

    // transit destination texture to "copy destination" state
    const D3D12_RESOURCE_STATES prevDestTextureState = mResourceStateCache.SetResourceState(destTex, 0, D3D12_RESOURCE_STATE_COPY_DEST);
    if (prevDestTextureState != D3D12_RESOURCE_STATE_COPY_DEST)
    {
        rb.Transition.pResource = destTex->GetResource();
        rb.Transition.StateBefore = prevDestTextureState;
        rb.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        barriers.PushBack(rb);
    }

    if (!barriers.Empty())
    {
        mCommandList->ResourceBarrier(barriers.Size(), barriers.Data());
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

    barriers.Clear();

    // transit source texture to previous state
    if (prevSrcTextureState != D3D12_RESOURCE_STATE_COPY_SOURCE)
    {
        rb.Transition.pResource = srcTex->GetResource();
        rb.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
        rb.Transition.StateAfter = prevSrcTextureState;
        barriers.PushBack(rb);

        mResourceStateCache.SetResourceState(srcTex, 0, prevSrcTextureState);
    }

    // transit destination texture to previous state
    if (prevDestTextureState != D3D12_RESOURCE_STATE_COPY_DEST)
    {
        rb.Transition.pResource = destTex->GetResource();
        rb.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        rb.Transition.StateAfter = prevDestTextureState;
        barriers.PushBack(rb);

        mResourceStateCache.SetResourceState(destTex, 0, prevDestTextureState);
    }

    if (!barriers.Empty())
    {
        mCommandList->ResourceBarrier(barriers.Size(), barriers.Data());
    }
}

void CommandRecorder::Clear(int flags, uint32 numTargets, const uint32* slots,
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

            const InternalTexturePtr& tex = mCurrRenderTarget->GetTexture(i);
            uint32 subResource = mCurrRenderTarget->GetSubresourceID(i);

            const auto currentResourceState = mResourceStateCache.GetResourceState(tex.Get(), subResource);
            NFE_ASSERT(currentResourceState == D3D12_RESOURCE_STATE_RENDER_TARGET, "Invalid resource state");

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

void CommandRecorder::Internal_UpdateStates()
{
    if (mCurrPipelineState != mPipelineState)
    {
        if (mBindingLayout == nullptr)
            mBindingLayout = mPipelineState->GetResBindingLayout().Get();
        else if (mBindingLayout != mPipelineState->GetResBindingLayout().Get())
            LOG_ERROR("Resource binding layout mismatch");

        // set pipeline state
        mCommandList->SetPipelineState(mPipelineState->GetPSO());
        mCurrPipelineState = mPipelineState;
        mCurrComputePipelineState = nullptr;

        // set root signature
        if (mCurrBindingLayout != mBindingLayout)
        {
            mCommandList->SetGraphicsRootSignature(mBindingLayout->mRootSignature.Get());
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

void CommandRecorder::Draw(int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    Internal_UpdateStates();
    mCommandList->DrawInstanced(vertexNum, instancesNum, vertexOffset, instanceOffset);
}

void CommandRecorder::DrawIndexed(int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    Internal_UpdateStates();
    mCommandList->DrawIndexedInstanced(indexNum, instancesNum, indexOffset, vertexOffset,
                                       instanceOffset);
}

void CommandRecorder::BindComputeResources(size_t slot, const ResourceBindingInstancePtr& bindingSetInstance)
{
    ResourceBindingInstance* instance = dynamic_cast<ResourceBindingInstance*>(bindingSetInstance.Get());
    if (!instance)
    {
        // clear the slot
        D3D12_GPU_DESCRIPTOR_HANDLE ptr;
        ptr.ptr = 0;
        mCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(slot), ptr);
        return;
    }

    Internal_ReferenceResource(bindingSetInstance);

    NFE_ASSERT(mComputeBindingLayout, "Compute binding layout not set");
    NFE_ASSERT(slot < mComputeBindingLayout->mBindingSets.size(), "Binding set index out of bounds");

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_GPU_DESCRIPTOR_HANDLE ptr = allocator.GetGpuHandle();
    ptr.ptr += instance->mDescriptorHeapOffset * allocator.GetDescriptorSize();
    mCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(slot), ptr);
}

void CommandRecorder::BindComputeVolatileCBuffer(size_t slot, const BufferPtr& buffer)
{
    NFE_ASSERT(slot < NFE_RENDERER_MAX_VOLATILE_CBUFFERS, "Invalid volatile buffer slot number");

    const Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(bufferPtr->GetMode() == BufferMode::Volatile, "Buffer mode must be volatile");

    mBoundComputeVolatileCBuffers[slot] = bufferPtr;
}

void CommandRecorder::SetComputeResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    ResourceBindingLayout* newLayout = dynamic_cast<ResourceBindingLayout*>(layout.Get());
    NFE_ASSERT(newLayout, "Invalid layout");

    Internal_ReferenceResource(layout);

    if (mComputeBindingLayout != newLayout)
    {
        mCommandList->SetComputeRootSignature(newLayout->GetD3DRootSignature());
        mComputeBindingLayout = newLayout;
    }
}

void CommandRecorder::SetComputePipelineState(const ComputePipelineStatePtr& state)
{
    ComputePipelineState* newState = dynamic_cast<ComputePipelineState*>(state.Get());
    NFE_ASSERT(newState, "Invalid compute pipeline state");

    Internal_ReferenceResource(state);

    if (mCurrComputePipelineState != newState)
    {
        mCommandList->SetPipelineState(newState->GetPSO());
        mCurrComputePipelineState = newState;
    }

    mCurrPipelineState = nullptr;
}

void CommandRecorder::Dispatch(uint32 x, uint32 y, uint32 z)
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

void CommandRecorder::BeginDebugGroup(const char* text)
{
    UNUSED(text);
}

void CommandRecorder::EndDebugGroup()
{
}

void CommandRecorder::InsertDebugMarker(const char* text)
{
    UNUSED(text);
}

} // namespace Renderer
} // namespace NFE
