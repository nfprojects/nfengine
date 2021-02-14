/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's command recorder
 */

#include "PCH.hpp"
#include "CommandRecorder.hpp"
#include "CommandListManager.hpp"
#include "CommandQueue.hpp"
#include "RendererD3D12.hpp"
#include "Buffer.hpp"
#include "RenderTarget.hpp"
#include "ComputePipelineState.hpp"
#include "Backbuffer.hpp"
#include "Translations.hpp"

#include "Engine/Common/Math/Vec4fU.hpp"
#include "Engine/Common/Containers/StaticArray.hpp"

namespace NFE {
namespace Renderer {

CommandRecorder::CommandRecorder()
    : mCommandList(nullptr)
    , mCommandListObject(nullptr)
    , mQueueType(CommandQueueType::Invalid)
{
    ResetState();
}

CommandRecorder::~CommandRecorder()
{
    NFE_ASSERT(mCommandList == nullptr, "Finish() not called before command buffer destruction");
}

bool CommandRecorder::Begin(CommandQueueType queueType)
{
    NFE_ASSERT(mCommandList == nullptr, "Command recorder is already in recording state");

    mResourceStateCache.OnBeginCommandBuffer();

    mQueueType = queueType;

    // get a free command list from the command list manager
    mCommandListObject = gDevice->GetCommandListManager()->RequestCommandList(queueType);
    if (!mCommandListObject)
    {
        NFE_LOG_ERROR("Failed to optain command list");
        return false;
    }
    NFE_ASSERT(mCommandListObject->GetState() == InternalCommandList::State::Recording, "Invalid command list state");

    mCommandList = mCommandListObject->GetD3DCommandList();
    NFE_ASSERT(mCommandList, "Invalid command list returned");

    if (mQueueType != CommandQueueType::Copy)
    {
        ID3D12DescriptorHeap* heaps[] =
        {
            gDevice->GetCbvSrvUavHeapAllocator().GetHeap(),
        };
        mCommandList->SetDescriptorHeaps(1, heaps);
    }

    ResetState();

    return true;
}

void CommandRecorder::ResourceBindingState::Reset()
{
    bindingLayoutChanged = false;
    bindingInstancesChanged = false;

    bindingLayout = nullptr;

    for (uint32 i = 0; i < NFE_RENDERER_MAX_VOLATILE_CBUFFERS; ++i)
    {
        volatileCBuffers[i] = nullptr;
    }

    for (uint32 i = 0; i < NFE_RENDERER_MAX_BINDING_SETS; ++i)
    {
        bindingInstances[i] = nullptr;
    }

    pendingDirectResourceBinds.Clear();
}

void CommandRecorder::ResetState()
{
    mGraphicsBindingState.Reset();
    mComputeBindingState.Reset();

    mCurrRenderTarget = nullptr;
    mGraphicsPipelineState = nullptr;
    mComputePipelineState = nullptr;

    mGraphicsPipelineStateChanged = false;
    mComputePipelineStateChanged = false;

    mCurrPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    mNumBoundVertexBuffers = 0u;
    mBoundIndexBuffer = nullptr;

    mVertexBufferChanged = false;
    mIndexBufferChanged = false;

    for (uint32 i = 0; i < NFE_RENDERER_MAX_VERTEX_BUFFERS; ++i)
    {
        mBoundVertexBuffers[i] = nullptr;
    }
}

CommandListPtr CommandRecorder::Finish()
{
    NFE_ASSERT(mCommandList, "Command buffer is not in recording state");

    Internal_UnsetRenderTarget();

    mResourceStateCache.FlushPendingBarriers(mCommandList);

    // pass resource states to the command list object
    mResourceStateCache.OnFinishCommandBuffer(mCommandListObject->mInitialResourceStates, mCommandListObject->mFinalResourceStates);

    mQueueType = CommandQueueType::Invalid;

    HRESULT hr = D3D_CALL_CHECK(mCommandList->Close());
    if (FAILED(hr))
    {
        // recording failed
        mCommandListObject = nullptr;
        mCommandList = nullptr;
        return nullptr;
    }

    CommandListPtr commandList = gDevice->GetCommandListManager()->OnCommandListRecorded(mCommandListObject);
    mCommandListObject = nullptr;
    mCommandList = nullptr;
    return commandList;
}

ReferencedResourcesList& CommandRecorder::Internal_GetReferencedResources()
{
    NFE_ASSERT(mCommandListObject, "Command buffer is not in recording state");
    return mCommandListObject->GetReferencedResources();
}


//////////////////////////////////////////////////////////////////////////

void CommandRecorder::SetViewport(float left, float width, float top, float height, float minDepth, float maxDepth)
{
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics, "Invalid queue type");

    D3D12_VIEWPORT viewport;
    viewport.TopLeftX = left;
    viewport.TopLeftY = top;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = minDepth;
    viewport.MaxDepth = maxDepth;
    mCommandList->RSSetViewports(1, &viewport);
}

void CommandRecorder::SetScissors(int32 left, int32 top, int32 right, int32 bottom)
{
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics, "Invalid queue type");

    D3D12_RECT rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    mCommandList->RSSetScissorRects(1, &rect);
}

void CommandRecorder::SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, uint32* strides, uint32* offsets)
{
    NFE_ASSERT(num < NFE_RENDERER_MAX_VERTEX_BUFFERS, "Too many vertex buffers");
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics, "Invalid queue type");

    for (uint32 i = 0; i < num; ++i)
    {
        D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = 0;
        uint32 size = 0;

        Buffer* buffer = static_cast<Buffer*>(vertexBuffers[i].Get());
        NFE_ASSERT(buffer, "Invalid vertex buffer at slot %i", i);

        if (buffer->GetResource())
        {
            Internal_GetReferencedResources().buffers.Insert(vertexBuffers[i]);

            gpuAddress = buffer->GetResource()->GetGPUVirtualAddress();
            size = buffer->GetSize();
        }

        mCurrVertexBufferViews[i].BufferLocation = gpuAddress + offsets[i];
        mCurrVertexBufferViews[i].SizeInBytes = size;
        mCurrVertexBufferViews[i].StrideInBytes = strides[i];

        mBoundVertexBuffers[i] = buffer;
    }

    mNumBoundVertexBuffers = static_cast<uint8>(num);
    mVertexBufferChanged = true;
}

void CommandRecorder::SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format)
{
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics, "Invalid queue type");

    mCurrIndexBufferView.BufferLocation = 0;
    mCurrIndexBufferView.SizeInBytes = 0;

    if (indexBuffer)
    {
        Internal_GetReferencedResources().buffers.Insert(indexBuffer);
        mBoundIndexBuffer = static_cast<Buffer*>(indexBuffer.Get());
        NFE_ASSERT(mBoundIndexBuffer, "Invalid index buffer");

        mCurrIndexBufferView.BufferLocation = mBoundIndexBuffer->GetResource()->GetGPUVirtualAddress();
        mCurrIndexBufferView.SizeInBytes = mBoundIndexBuffer->GetSize();
        switch (format)
        {
        case IndexBufferFormat::Uint16:
            mCurrIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
            break;
        case IndexBufferFormat::Uint32:
            mCurrIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
            break;
        default:
            NFE_FATAL("Invalid index buffer format");
        };
    }
    else
    {
        mBoundIndexBuffer = nullptr;
    }

    mIndexBufferChanged = true;
}

void CommandRecorder::BindResources(PipelineType pipelineType, uint32 setIndex, const ResourceBindingInstancePtr& bindingSetInstance)
{
    ResourceBindingState& bindingState = GetBindingState(pipelineType);

    NFE_ASSERT(setIndex < NFE_RENDERER_MAX_BINDING_SETS, "Invalid binding slot");
    NFE_ASSERT(bindingState.bindingLayout, "Binding layout is not set");
    NFE_ASSERT(setIndex < bindingState.bindingLayout->mBindingSets.Size(), "Binding set index out of bounds");

    ResourceBindingInstance* instance = static_cast<ResourceBindingInstance*>(bindingSetInstance.Get());

    if (instance)
    {
        NFE_ASSERT(instance->IsFinalized(), "Binding instance is not finalized");
    }

    if (instance != bindingState.bindingInstances[setIndex])
    {
        bindingState.bindingInstances[setIndex] = instance;
        Internal_ReferenceBindingSetInstance(bindingSetInstance);
        bindingState.bindingInstancesChanged = true;
    }

    // TODO clear direct bindings to that set
}

void CommandRecorder::BindTexture(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const TexturePtr& texture, const TextureView& view)
{
    ResourceBindingState& bindingState = GetBindingState(pipelineType);

    Texture* texturePtr = static_cast<Texture*>(texture.Get());
    NFE_ASSERT(texturePtr, "Invalid texture");

    NFE_ASSERT(setIndex < NFE_RENDERER_MAX_BINDING_SETS, "Invalid binding slot");
    NFE_ASSERT(bindingState.bindingLayout, "Binding layout is not set");
    NFE_ASSERT(setIndex < bindingState.bindingLayout->mBindingSets.Size(), "Binding set index out of bounds");
    
    const InternalResourceBindingSetPtr& bindingSet = bindingState.bindingLayout->mBindingSets[setIndex];
    NFE_ASSERT(slotInSet < bindingSet->mBindings.Size(), "Binding slot index within set is out of bounds");

    Internal_GetReferencedResources().textures.Insert(texture);

    PendingDirectResourceBind resourceBind;
    resourceBind.setIndex = setIndex;
    resourceBind.slotInSet = slotInSet;
    resourceBind.type = ResourceType::Texture;
    resourceBind.shaderWritable = false;
    resourceBind.texture = texturePtr;
    resourceBind.textureView = view;

    bindingState.pendingDirectResourceBinds.PushBack(std::move(resourceBind));
}

void CommandRecorder::BindWritableTexture(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const TexturePtr& texture, const TextureView& view)
{
    ResourceBindingState& bindingState = GetBindingState(pipelineType);

    Texture* texturePtr = static_cast<Texture*>(texture.Get());
    NFE_ASSERT(texturePtr, "Invalid texture");

    NFE_ASSERT(setIndex < NFE_RENDERER_MAX_BINDING_SETS, "Invalid binding slot");
    NFE_ASSERT(bindingState.bindingLayout, "Binding layout is not set");
    NFE_ASSERT(setIndex < bindingState.bindingLayout->mBindingSets.Size(), "Binding set index out of bounds");

    const InternalResourceBindingSetPtr& bindingSet = bindingState.bindingLayout->mBindingSets[setIndex];
    NFE_ASSERT(slotInSet < bindingSet->mBindings.Size(), "Binding slot index within set is out of bounds");

    Internal_GetReferencedResources().textures.Insert(texture);

    PendingDirectResourceBind resourceBind;
    resourceBind.setIndex = setIndex;
    resourceBind.slotInSet = slotInSet;
    resourceBind.type = ResourceType::Texture;
    resourceBind.shaderWritable = true;
    resourceBind.texture = texturePtr;
    resourceBind.textureView = view;

    bindingState.pendingDirectResourceBinds.PushBack(std::move(resourceBind));
}

void CommandRecorder::BindBuffer(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const BufferPtr& buffer, const BufferView& view)
{
    ResourceBindingState& bindingState = GetBindingState(pipelineType);

    Buffer* bufferPtr = static_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(bufferPtr, "Invalid buffer");

    NFE_ASSERT(setIndex < NFE_RENDERER_MAX_BINDING_SETS, "Invalid binding slot");
    NFE_ASSERT(bindingState.bindingLayout, "Binding layout is not set");
    NFE_ASSERT(setIndex < bindingState.bindingLayout->mBindingSets.Size(), "Binding set index out of bounds");

    const InternalResourceBindingSetPtr& bindingSet = bindingState.bindingLayout->mBindingSets[setIndex];
    NFE_ASSERT(slotInSet < bindingSet->mBindings.Size(), "Binding slot index within set is out of bounds");

    Internal_GetReferencedResources().buffers.Insert(buffer);

    PendingDirectResourceBind resourceBind;
    resourceBind.setIndex = setIndex;
    resourceBind.slotInSet = slotInSet;
    resourceBind.type = ResourceType::Buffer;
    resourceBind.shaderWritable = false;
    resourceBind.buffer = bufferPtr;
    resourceBind.bufferView = view;

    bindingState.pendingDirectResourceBinds.PushBack(std::move(resourceBind));
}

void CommandRecorder::BindWritableBuffer(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const BufferPtr& buffer, const BufferView& view)
{
    ResourceBindingState& bindingState = GetBindingState(pipelineType);

    Buffer* bufferPtr = static_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(bufferPtr, "Invalid buffer");

    NFE_ASSERT(setIndex < NFE_RENDERER_MAX_BINDING_SETS, "Invalid binding slot");
    NFE_ASSERT(bindingState.bindingLayout, "Binding layout is not set");
    NFE_ASSERT(setIndex < bindingState.bindingLayout->mBindingSets.Size(), "Binding set index out of bounds");

    const InternalResourceBindingSetPtr& bindingSet = bindingState.bindingLayout->mBindingSets[setIndex];
    NFE_ASSERT(slotInSet < bindingSet->mBindings.Size(), "Binding slot index within set is out of bounds");

    Internal_GetReferencedResources().buffers.Insert(buffer);

    PendingDirectResourceBind resourceBind;
    resourceBind.setIndex = setIndex;
    resourceBind.slotInSet = slotInSet;
    resourceBind.type = ResourceType::Buffer;
    resourceBind.shaderWritable = true;
    resourceBind.buffer = bufferPtr;
    resourceBind.bufferView = view;

    bindingState.pendingDirectResourceBinds.PushBack(std::move(resourceBind));
}

void CommandRecorder::BindVolatileCBuffer(PipelineType pipelineType, uint32 slot, const BufferPtr& buffer)
{
    ResourceBindingState& bindingState = GetBindingState(pipelineType);

    NFE_ASSERT(slot < NFE_RENDERER_MAX_VOLATILE_CBUFFERS, "Invalid volatile buffer slot number");

    const Buffer* bufferPtr = static_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(bufferPtr->GetMode() == ResourceAccessMode::Volatile, "Buffer mode must be volatile");

    bindingState.volatileCBuffers[slot] = bufferPtr;
}

void CommandRecorder::SetRenderTarget(const RenderTargetPtr& renderTarget)
{
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics, "Invalid queue type");

    if (mCurrRenderTarget == renderTarget.Get())
        return;

    Internal_UnsetRenderTarget();

    mCurrRenderTarget = static_cast<RenderTarget*>(renderTarget.Get());
    if (!mCurrRenderTarget)
    {
        return;
    }

    Internal_GetReferencedResources().renderTargets.Insert(renderTarget);

    HeapAllocator& allocator = gDevice->GetRtvHeapAllocator();
    HeapAllocator& dsvAllocator = gDevice->GetDsvHeapAllocator();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[MAX_RENDER_TARGETS];
    D3D12_CPU_DESCRIPTOR_HANDLE dsv;
    bool setDsv = false;

    const uint32 numTargets = mCurrRenderTarget->GetNumTargets();
    for (uint32 i = 0; i < numTargets; ++i)
    {
        const InternalTexturePtr& tex = mCurrRenderTarget->GetTexture(i);
        uint32 subResource = mCurrRenderTarget->GetSubresourceID(i);

        rtvs[i] = allocator.GetCpuHandle();
        rtvs[i].ptr += (uint64)mCurrRenderTarget->GetRTV(i) * allocator.GetDescriptorSize();

        mResourceStateCache.EnsureResourceState(tex.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, subResource);
    }

    if (mCurrRenderTarget->GetDSV() != -1)
    {
        const InternalTexturePtr& tex = mCurrRenderTarget->GetDepthTexture();
        uint32 subResource = mCurrRenderTarget->GetDepthTexSubresourceID();

        mResourceStateCache.EnsureResourceState(tex.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, subResource);

        dsv = dsvAllocator.GetCpuHandle();
        dsv.ptr += (uint64)mCurrRenderTarget->GetDSV() * dsvAllocator.GetDescriptorSize();
        setDsv = true;
    }

    mCommandList->OMSetRenderTargets(static_cast<UINT>(numTargets), rtvs, FALSE, setDsv ? &dsv : nullptr);
}

void CommandRecorder::Internal_UnsetRenderTarget()
{
    mCurrRenderTarget = nullptr;
}

void CommandRecorder::SetResourceBindingLayout(PipelineType pipelineType, const ResourceBindingLayoutPtr& layout)
{
    ResourceBindingState& bindingState = GetBindingState(pipelineType);

    if (layout)
    {
        Internal_GetReferencedResources().bindingSetLayouts.Insert(layout);
    }

    ResourceBindingLayout* layoutPtr = static_cast<ResourceBindingLayout*>(layout.Get());
    NFE_ASSERT(layoutPtr, "Invalid layout");

    if (layoutPtr != bindingState.bindingLayout)
    {
        bindingState.bindingLayout = layoutPtr;
        bindingState.bindingLayoutChanged = true;
    }
}

void CommandRecorder::SetPipelineState(const PipelineStatePtr& state)
{
    if (state)
    {
        Internal_GetReferencedResources().pipelineStates.Insert(state);
    }

    PipelineState* pipelineStatePtr = static_cast<PipelineState*>(state.Get());
    NFE_ASSERT(pipelineStatePtr, "Invalid pipeline state");

    if (pipelineStatePtr != mGraphicsPipelineState)
    {
        mGraphicsPipelineState = pipelineStatePtr;
        mGraphicsPipelineStateChanged = true;
    }
}

void CommandRecorder::SetStencilRef(uint8 ref)
{
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics, "Invalid queue type");

    mCommandList->OMSetStencilRef(ref);
}

void CommandRecorder::Internal_WriteDynamicBuffer(Buffer* buffer, size_t offset, size_t size, const void* data)
{
    RingBuffer* ringBuffer = gDevice->GetRingBuffer();
    NFE_ASSERT(ringBuffer, "No ring buffer");

    size_t ringBufferOffset = ringBuffer->Allocate(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    NFE_ASSERT(ringBufferOffset != RingBuffer::INVALID_OFFSET, "Ring buffer allocation failed");

    // copy data from CPU memory to staging ring buffer
    char* cpuPtr = reinterpret_cast<char*>(ringBuffer->GetCpuAddress());
    cpuPtr += ringBufferOffset;
    memcpy(cpuPtr, data, size);

    mResourceStateCache.EnsureResourceState(buffer, D3D12_RESOURCE_STATE_COPY_DEST);
    mResourceStateCache.FlushPendingBarriers(mCommandList);

    // copy data from staging ring buffer to the target buffer
    mCommandList->CopyBufferRegion(buffer->GetResource(), static_cast<UINT64>(offset),
                                   ringBuffer->GetD3DResource(), static_cast<UINT64>(ringBufferOffset),
                                   static_cast<UINT64>(size));
}

void CommandRecorder::Internal_WriteVolatileBuffer(Buffer* buffer, const void* data)
{
    RingBuffer* ringBuffer = gDevice->GetRingBuffer();
    NFE_ASSERT(ringBuffer, "No ring buffer");

    size_t ringBufferOffset = ringBuffer->Allocate(buffer->GetSize(), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    NFE_ASSERT(ringBufferOffset != RingBuffer::INVALID_OFFSET, "Ring buffer allocation failed");

    // copy data from CPU memory to ring buffer
    char* cpuPtr = reinterpret_cast<char*>(ringBuffer->GetCpuAddress());
    cpuPtr += ringBufferOffset;
    memcpy(cpuPtr, data, buffer->GetSize());

    D3D12_GPU_VIRTUAL_ADDRESS gpuPtr = ringBuffer->GetGpuAddress();
    gpuPtr += ringBufferOffset;

    Internal_UpdateGraphicsResourceBindingLayout();
    Internal_UpdateComputeResourceBindingLayout();

    // check if the buffer is not used as CBV
    for (uint32 i = 0; i < NFE_RENDERER_MAX_VOLATILE_CBUFFERS; ++i)
    {
        if (mGraphicsBindingState.volatileCBuffers[i] == buffer)
        {
            const uint32 rootParamIndex = mGraphicsBindingState.bindingLayout->mBindingSets.Size() + i;
            mCommandList->SetGraphicsRootConstantBufferView(rootParamIndex, gpuPtr);
        }

        if (mComputeBindingState.volatileCBuffers[i] == buffer)
        {
            const uint32 rootParamIndex = mComputeBindingState.bindingLayout->mBindingSets.Size() + i;
            mCommandList->SetComputeRootConstantBufferView(rootParamIndex, gpuPtr);
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
    Buffer* bufferPtr = static_cast<Buffer*>(buffer.Get());
    NFE_ASSERT(bufferPtr, "Invalid buffer");

    Internal_GetReferencedResources().buffers.Insert(buffer);

    if (bufferPtr->GetMode() == ResourceAccessMode::GPUOnly)
    {
        if (size > bufferPtr->GetSize())
        {
            NFE_LOG_ERROR("Trying to perform write bigger than buffer size.");
            return false;
        }

        Internal_WriteDynamicBuffer(bufferPtr, offset, size, data);
    }
    else if (bufferPtr->GetMode() == ResourceAccessMode::Volatile)
    {
        NFE_ASSERT(offset == 0, "Offset not supported");
        NFE_ASSERT(size == bufferPtr->GetSize(), "Size must cover the whole buffer");

        Internal_WriteVolatileBuffer(bufferPtr, data);
    }
    else
    {
        NFE_FATAL("Specified buffer can not be CPU-written");
        return false;
    }

    return true;
}

void CommandRecorder::CopyBuffer(const BufferPtr& src, const BufferPtr& dest, size_t size, size_t srcOffset, size_t destOffset)
{
    Buffer* srcBuffer = static_cast<Buffer*>(src.Get());
    NFE_ASSERT(srcBuffer, "Invalid buffer");

    Buffer* destBuffer = static_cast<Buffer*>(dest.Get());
    NFE_ASSERT(destBuffer, "Invalid buffer");

    Internal_GetReferencedResources().buffers.Insert(src);
    Internal_GetReferencedResources().buffers.Insert(dest);

    if (srcBuffer->GetMode() != ResourceAccessMode::Upload && srcBuffer->GetMode() != ResourceAccessMode::GPUOnly)
    {
        NFE_FATAL("Invalid source buffer access mode");
    }

    if (destBuffer->GetMode() != ResourceAccessMode::GPUOnly && destBuffer->GetMode() != ResourceAccessMode::Readback)
    {
        NFE_FATAL("Invalid destination buffer access mode");
    }

    if (size == 0)
    {
        size = srcBuffer->GetSize();
    }

    NFE_ASSERT(srcOffset + size <= srcBuffer->GetSize(), "Out of source buffer range: offset=%u, readSize=%u, bufferSize=%u", srcOffset, size, srcBuffer->GetSize());
    NFE_ASSERT(destOffset + size <= destBuffer->GetSize(), "Out of destination buffer range: offset=%u, readSize=%u, bufferSize=%u", destOffset, size, destBuffer->GetSize());

    if (srcBuffer->GetMode() == ResourceAccessMode::GPUOnly)
    {
        mResourceStateCache.EnsureResourceState(srcBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
    }
    if (destBuffer->GetMode() == ResourceAccessMode::GPUOnly)
    {
        mResourceStateCache.EnsureResourceState(destBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
    }
    mResourceStateCache.FlushPendingBarriers(mCommandList);

    mCommandList->CopyBufferRegion(
        destBuffer->GetResource(), srcOffset,
        srcBuffer->GetResource(), destOffset,
        size);
}

bool CommandRecorder::WriteTexture(const TexturePtr& texture, const void* data, const TextureRegion* texRegion, uint32 srcRowStride)
{
    Texture* texturePtr = static_cast<Texture*>(texture.Get());
    NFE_ASSERT(texturePtr, "Invalid texture");
    NFE_ASSERT(texturePtr->GetMode() == ResourceAccessMode::GPUOnly, "Invalid texture type");

    uint32 targetMipmap = 0u, targetLayer = 0u;
    uint32 writeX = 0u, writeY = 0u, writeZ = 0u;
    uint32 writeWidth = texturePtr->GetWidth();
    uint32 writeHeight = texturePtr->GetHeight();
    uint32 writeDepth = texturePtr->GetDepth();

    if (texRegion)
    {
        targetMipmap = texRegion->mipmap;
        targetLayer = texRegion->layer;

        writeX = texRegion->x;
        writeY = texRegion->y;
        writeZ = texRegion->z;

        writeWidth = texRegion->width;
        writeHeight = texRegion->height;
        writeDepth = texRegion->depth;

        NFE_ASSERT(
            writeWidth > 0u && writeHeight >= 0u && writeDepth >= 0u,
            "Invalid texture region");

        NFE_ASSERT(
            writeX < texturePtr->GetWidth() &&
            writeY < texturePtr->GetHeight() &&
            writeZ < texturePtr->GetDepth(),
            "Invalid texture region");

        NFE_ASSERT(
            writeX + writeWidth <= texturePtr->GetWidth() &&
            writeY + writeHeight <= texturePtr->GetHeight() &&
            writeZ + writeDepth <= texturePtr->GetDepth(),
            "Invalid texture region");
    }

    // validate subresource index
    NFE_ASSERT(targetMipmap < texturePtr->GetMipmapsNum(), "Invalid mipmap");
    NFE_ASSERT(targetLayer < texturePtr->GetLayersNum(), "Invalid mipmap");

    const uint32 subresourceIndex = targetMipmap + targetLayer * texturePtr->GetMipmapsNum();

    // compute src row size & stride
    const uint32 srcRowSize = writeWidth * GetElementFormatSize(texturePtr->GetFormat()); 
    if (srcRowStride > 0u)
    {
        NFE_ASSERT(srcRowStride >= srcRowSize, "Row stride is too small");
    }
    else
    {
        srcRowStride = srcRowSize;
    }

    // compute data layout of source texture data
    D3D12_SUBRESOURCE_FOOTPRINT subresourceFootprint;
    subresourceFootprint.Format = TranslateElementFormat(texturePtr->GetFormat());
    subresourceFootprint.Width = writeWidth;
    subresourceFootprint.Height = writeHeight;
    subresourceFootprint.Depth = writeDepth;
    subresourceFootprint.RowPitch = Math::RoundUp<uint32>(srcRowSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

    RingBuffer* ringBuffer = gDevice->GetRingBuffer();
    NFE_ASSERT(ringBuffer, "No ring buffer");

    size_t ringBufferOffset = 0;
    {
        // texture data must be aligned to 512 bytes
        const size_t totalBytes = (size_t)writeDepth * (size_t)writeHeight * (size_t)subresourceFootprint.RowPitch;
        ringBufferOffset = ringBuffer->Allocate(totalBytes, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
        NFE_ASSERT(ringBufferOffset != RingBuffer::INVALID_OFFSET, "Ring buffer allocation failed");

        // copy data from CPU memory to staging ring buffer
        uint8* destCpuPtr = reinterpret_cast<uint8*>(ringBuffer->GetCpuAddress()) + ringBufferOffset;
        const uint8* srcCpuPtr = reinterpret_cast<const uint8*>(data);

        for (uint32 z = 0; z < writeDepth; ++z)
        {
            for (uint32 y = 0; y < writeHeight; ++y)
            {
                memcpy(destCpuPtr, srcCpuPtr, srcRowSize);
                destCpuPtr += subresourceFootprint.RowPitch;
                srcCpuPtr += srcRowStride;
            }
        }
    }

    Internal_GetReferencedResources().textures.Insert(texture);
    mResourceStateCache.EnsureResourceState(texturePtr, D3D12_RESOURCE_STATE_COPY_DEST, subresourceIndex);
    mResourceStateCache.FlushPendingBarriers(mCommandList);

    D3D12_TEXTURE_COPY_LOCATION src;
    src.pResource = ringBuffer->GetD3DResource();
    src.PlacedFootprint.Footprint = subresourceFootprint;
    src.PlacedFootprint.Offset = ringBufferOffset;
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

    D3D12_TEXTURE_COPY_LOCATION dest;
    dest.pResource = texturePtr->GetD3DResource();
    dest.SubresourceIndex = subresourceIndex;
    dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

    // copy data from staging ring buffer to the target texture
    mCommandList->CopyTextureRegion(&dest, writeX, writeY, writeZ, &src, nullptr);

    return true;
}

void CommandRecorder::CopyTextureToBuffer(const TexturePtr& src, const BufferPtr& dest, const TextureRegion* texRegion, uint32 bufferOffset)
{
    NFE_ASSERT(src, "Invalid source texture");
    NFE_ASSERT(dest, "Invalid destination buffer");

    Internal_GetReferencedResources().textures.Insert(src);
    Internal_GetReferencedResources().buffers.Insert(dest);

    const Texture* srcTex = dynamic_cast<const Texture*>(src.Get());
    NFE_ASSERT(src, "Invalid 'src' pointer");

    const Buffer* destBuf = dynamic_cast<const Buffer*>(dest.Get());
    NFE_ASSERT(destBuf, "Invalid 'dest' pointer");

    if (destBuf->GetMode() == ResourceAccessMode::Upload || destBuf->GetMode() == ResourceAccessMode::Volatile)
    {
        NFE_LOG_ERROR("Can't copy texture to this kind of buffer");
        return;
    }

    uint32 mipmap = 0u, layer = 0u;

    D3D12_BOX srcBox;
    srcBox.left = 0u;
    srcBox.top = 0u;
    srcBox.front = 0;
    srcBox.right = srcTex->GetWidth();
    srcBox.bottom = srcTex->GetHeight();
    srcBox.back = srcTex->GetDepth();

    if (texRegion)
    {
        mipmap = texRegion->mipmap;
        layer = texRegion->layer;

        srcBox.left = texRegion->x;
        srcBox.top = texRegion->y;
        srcBox.front = texRegion->z;

        srcBox.right = texRegion->x + texRegion->width;
        srcBox.bottom = texRegion->y + texRegion->height;
        srcBox.back = texRegion->z + texRegion->depth;

        NFE_ASSERT(
            srcBox.left < srcTex->GetWidth() &&
            srcBox.top < srcTex->GetHeight() &&
            srcBox.front < srcTex->GetDepth(),
            "Invalid texture region");

        NFE_ASSERT(
            srcBox.right <= srcTex->GetWidth() &&
            srcBox.bottom <= srcTex->GetHeight() &&
            srcBox.back <= srcTex->GetDepth(),
            "Invalid texture region");
    }

    // validate subresource index
    NFE_ASSERT(mipmap < srcTex->GetMipmapsNum(), "Invalid mipmap");
    NFE_ASSERT(layer < srcTex->GetLayersNum(), "Invalid mipmap");

    const uint32 subresourceIndex = mipmap + layer * srcTex->GetMipmapsNum();
    const uint32 srcRowSize = (srcBox.right - srcBox.left) * GetElementFormatSize(srcTex->GetFormat());

    mResourceStateCache.EnsureResourceState(srcTex, D3D12_RESOURCE_STATE_COPY_SOURCE, subresourceIndex);
    mResourceStateCache.EnsureResourceState(destBuf, D3D12_RESOURCE_STATE_COPY_DEST);
    mResourceStateCache.FlushPendingBarriers(mCommandList);

    // compute data layout of source texture data
    D3D12_SUBRESOURCE_FOOTPRINT subresourceFootprint;
    subresourceFootprint.Format = TranslateElementFormat(srcTex->GetFormat());
    subresourceFootprint.Width = srcBox.right - srcBox.left;
    subresourceFootprint.Height = srcBox.bottom - srcBox.top;
    subresourceFootprint.Depth = srcBox.back - srcBox.front;
    subresourceFootprint.RowPitch = Math::RoundUp<uint32>(srcRowSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

    // validate buffer
    const size_t bufferRequiredSize = (size_t)subresourceFootprint.RowPitch * (size_t)subresourceFootprint.Height * (size_t)subresourceFootprint.Depth;
    NFE_ASSERT(bufferOffset + bufferRequiredSize <= destBuf->GetSize(),
        "Target buffer is too small to perform the copy: offset=%u, texDataSize=%zu, bufferSize=%u", bufferOffset, bufferRequiredSize, destBuf->GetSize());
    NFE_ASSERT(bufferOffset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT == 0,
        "Invalid buffer offset: %u. Must be aligned to %u", bufferOffset, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

    D3D12_TEXTURE_COPY_LOCATION destLocation;
    destLocation.pResource = destBuf->GetD3DResource();
    destLocation.PlacedFootprint.Footprint = subresourceFootprint;
    destLocation.PlacedFootprint.Offset = bufferOffset;
    destLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    srcLocation.pResource = srcTex->GetD3DResource();
    srcLocation.SubresourceIndex = subresourceIndex;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

    mCommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, &srcBox);
}

void CommandRecorder::CopyTexture(const TexturePtr& src, const TexturePtr& dest)
{
    NFE_ASSERT(src, "Invalid source texture");
    NFE_ASSERT(dest, "Invalid destination texture");

    Internal_GetReferencedResources().textures.Insert(src);
    Internal_GetReferencedResources().textures.Insert(dest);

    Texture* srcTex = dynamic_cast<Texture*>(src.Get());
    NFE_ASSERT(src, "Invalid 'src' pointer");

    if (srcTex->GetMode() == ResourceAccessMode::Readback || srcTex->GetMode() == ResourceAccessMode::Volatile)
    {
        NFE_LOG_ERROR("Can't copy from this texture");
        return;
    }

    Texture* destTex = dynamic_cast<Texture*>(dest.Get());
    NFE_ASSERT(destTex, "Invalid 'dest' pointer");

    if (destTex->GetMode() == ResourceAccessMode::Volatile)
    {
        NFE_LOG_ERROR("Can't copy to this texture");
        return;
    }

    mResourceStateCache.EnsureResourceState(srcTex, D3D12_RESOURCE_STATE_COPY_SOURCE);
    mResourceStateCache.EnsureResourceState(destTex, D3D12_RESOURCE_STATE_COPY_DEST);
    mResourceStateCache.FlushPendingBarriers(mCommandList);

    // perform copy
    if (destTex->GetMode() == ResourceAccessMode::Readback)
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
}

void CommandRecorder::CopyTexture(const TexturePtr& src, const BackbufferPtr& dest)
{
    NFE_ASSERT(src, "Invalid source texture");
    NFE_ASSERT(dest, "Invalid destination backbuffer");

    Internal_GetReferencedResources().textures.Insert(src);

    Texture* srcTex = dynamic_cast<Texture*>(src.Get());
    NFE_ASSERT(src, "Invalid 'src' pointer");

    Backbuffer* destBackbuffer = static_cast<Backbuffer*>(dest.Get());
    NFE_ASSERT(src, "Invalid 'src' pointer");

    if (srcTex->GetMode() == ResourceAccessMode::Readback || srcTex->GetMode() == ResourceAccessMode::Volatile)
    {
        NFE_LOG_ERROR("Can't copy from this texture");
        return;
    }

    Backbuffer* backbuffer = dynamic_cast<Backbuffer*>(dest.Get());
    NFE_ASSERT(backbuffer, "Invalid 'dest' pointer");

    const D3D12_RESOURCE_STATES srcTextureState = srcTex->GetSamplesNum() == 1 ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
    const D3D12_RESOURCE_STATES destTextureState = srcTex->GetSamplesNum() == 1 ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_RESOLVE_DEST;

    mResourceStateCache.EnsureResourceState(srcTex, srcTextureState);
    mResourceStateCache.EnsureResourceState(backbuffer, destTextureState);
    mResourceStateCache.FlushPendingBarriers(mCommandList);

    // perform copy
    if (srcTex->GetSamplesNum() == 1)
    {
        mCommandList->CopyResource(backbuffer->GetCurrentBuffer(), srcTex->GetResource());
    }
    else
    {
        mCommandList->ResolveSubresource(backbuffer->GetCurrentBuffer(), 0, srcTex->GetResource(), 0, destBackbuffer->GetFormat());
    }

    // a bit hacky
    mResourceStateCache.EnsureResourceState(backbuffer, D3D12_RESOURCE_STATE_PRESENT);
}

void CommandRecorder::Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Vec4fU* colors, float depthValue, uint8 stencilValue)
{
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics, "Invalid queue type");

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
                    NFE_LOG_ERROR("Invalid render target texture slot = %u", slots[i]);
                    return;
                }

                slot = slots[i];
            }

            const InternalTexturePtr& tex = mCurrRenderTarget->GetTexture(i);
            uint32 subResource = mCurrRenderTarget->GetSubresourceID(i);

            mResourceStateCache.EnsureResourceState(tex.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, subResource);
            mResourceStateCache.FlushPendingBarriers(mCommandList);

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

void CommandRecorder::Internal_UpdateComputePipelineState()
{
    if (mComputePipelineStateChanged)
    {
        mComputePipelineStateChanged = false;

        mCommandList->SetPipelineState(mComputePipelineState->GetPSO());
    }

    // setting compute pipeline should override graphics pipeline
    mGraphicsPipelineStateChanged = true;
}

void CommandRecorder::Internal_UpdateGraphicsPipelineState()
{
    if (mGraphicsPipelineStateChanged)
    {
        mGraphicsPipelineStateChanged = false;

        mCommandList->SetPipelineState(mGraphicsPipelineState->GetPSO());

        // set primitive type
        D3D_PRIMITIVE_TOPOLOGY newTopology = mGraphicsPipelineState->GetPrimitiveTopology();
        if (newTopology != mCurrPrimitiveTopology)
        {
            mCurrPrimitiveTopology = newTopology;
            mCommandList->IASetPrimitiveTopology(newTopology);
        }
    }

    // setting graphics pipeline should override compute pipeline
    mComputePipelineStateChanged = true;
}

void CommandRecorder::Internal_UpdateVetexAndIndexBuffers()
{
    if (mIndexBufferChanged)
    {
        if (mBoundIndexBuffer)
        {
            if (mBoundIndexBuffer->GetResource())
            {
                mResourceStateCache.EnsureResourceState(mBoundIndexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
            }
        }

        mCommandList->IASetIndexBuffer(&mCurrIndexBufferView);
        mIndexBufferChanged = false;
    }

    if (mVertexBufferChanged)
    {
        for (uint32 i = 0; i < mNumBoundVertexBuffers; ++i)
        {
            const Buffer* buffer = mBoundVertexBuffers[i];
            if (buffer->GetResource())
            {
                mResourceStateCache.EnsureResourceState(mBoundVertexBuffers[i], D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            }
        }

        mCommandList->IASetVertexBuffers(0, mNumBoundVertexBuffers, mCurrVertexBufferViews);
        mVertexBufferChanged = false;
    }
}

void CommandRecorder::Internal_UpdateGraphicsResourceBindingLayout()
{
    if (mGraphicsBindingState.bindingLayoutChanged)
    {
        mGraphicsBindingState.bindingLayoutChanged = false;

        mCommandList->SetGraphicsRootSignature(mGraphicsBindingState.bindingLayout->mRootSignature.Get());
    }
}

HeapAllocator::DescriptorRange CommandRecorder::Internal_GenerateDescriptorTableOverride(ResourceBindingState& state, uint32 setIndex, const ResourceBindingInstance* bindingInstance)
{
    NFE_ASSERT(state.bindingLayout, "Invalid layout");
    const uint32 numResourcesInSet = state.bindingLayout->mBindingSets[setIndex]->GetNumResources();

    HeapAllocator& stagingHeapAllocator = gDevice->GetCbvSrvUavHeapStagingAllocator();
    HeapAllocator& heapAllocator = gDevice->GetCbvSrvUavHeapAllocator();

    // allocate descriptor table for the override
    // TODO instead of using generic allocator, maybe switch to ring buffer for temporary descriptors
    HeapAllocator::DescriptorRange descriptorRange = heapAllocator.Allocate(numResourcesInSet);
    NFE_ASSERT(descriptorRange.offset != UINT32_MAX, "Descriptor table allocation failed");

    // copy descriptors from existing binding instance
    if (bindingInstance)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = stagingHeapAllocator.GetCpuHandle();
        srcHandle.ptr += stagingHeapAllocator.GetDescriptorSize() * static_cast<size_t>(bindingInstance->mCpuDescriptorHeapOffset);

        D3D12_CPU_DESCRIPTOR_HANDLE destHandle = heapAllocator.GetCpuHandle();
        destHandle.ptr += heapAllocator.GetDescriptorSize() * static_cast<size_t>(descriptorRange.offset);

        gDevice->GetDevice()->CopyDescriptorsSimple(numResourcesInSet, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    }

    // create descriptors for the overrides
    for (const PendingDirectResourceBind& resourceBind : state.pendingDirectResourceBinds)
    {
        if (resourceBind.setIndex == setIndex)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE targetDescriptor = heapAllocator.GetCpuHandle();
            targetDescriptor.ptr += heapAllocator.GetDescriptorSize() * static_cast<size_t>(descriptorRange.offset + resourceBind.slotInSet);

            if (resourceBind.type == ResourceType::Texture)
            {
                if (resourceBind.shaderWritable)
                {
                    CreateTextureUAV(resourceBind.texture, resourceBind.textureView, targetDescriptor);

                    mResourceStateCache.EnsureResourceState(resourceBind.texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                }
                else
                {
                    CreateTextureSRV(resourceBind.texture, resourceBind.textureView, targetDescriptor);

                    // TODO limit shader visibility (based on binding layout properties)
                    mResourceStateCache.EnsureResourceState(resourceBind.texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                }
            }
            else if(resourceBind.type == ResourceType::Buffer)
            {
                if (resourceBind.shaderWritable)
                {
                    CreateBufferUAV(resourceBind.buffer, resourceBind.bufferView, targetDescriptor);

                    mResourceStateCache.EnsureResourceState(resourceBind.buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                }
                else
                {
                    CreateBufferSRV(resourceBind.buffer, resourceBind.bufferView, targetDescriptor);

                    // TODO limit shader visibility (based on binding layout properties)
                    mResourceStateCache.EnsureResourceState(resourceBind.buffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                }
            }
            else
            {
                NFE_FATAL("Invalid resource type");
            }
        }
    }

    // add to referenced list so it will be released automatically later
    mCommandListObject->GetReferencedDescriptorsRanges().PushBack(descriptorRange);

    // TODO verify if all resources in the set are bound, if not - assert, because it may crash the driver
    // TODO ensure proper resource states (the non-overwritten ones)

    return descriptorRange;
}

void CommandRecorder::Internal_UpdateResourceBindings(PipelineType pipelineType)
{
    ResourceBindingState& state = GetBindingState(pipelineType);

    if (!state.bindingInstancesChanged && state.pendingDirectResourceBinds.Empty())
    {
        return;
    }

    // determine, which sets require custom descriptor table
    uint32 directBindingSetMask = 0;
    for (const PendingDirectResourceBind& resourceBind : state.pendingDirectResourceBinds)
    {
        directBindingSetMask |= 1u << resourceBind.setIndex;
    }

    for (uint32 setIndex = 0; setIndex < NFE_RENDERER_MAX_BINDING_SETS; ++setIndex)
    {
        const ResourceBindingInstance* instance = state.bindingInstances[setIndex];

        if (1u == ((directBindingSetMask >> setIndex) & 1u))
        {
            HeapAllocator::DescriptorRange descriptorRange = Internal_GenerateDescriptorTableOverride(state, setIndex, instance);

            HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
            D3D12_GPU_DESCRIPTOR_HANDLE ptr = allocator.GetGpuHandle();
            ptr.ptr += (uint64)descriptorRange.offset * allocator.GetDescriptorSize();

            if (pipelineType == PipelineType::Compute)
            {
                mCommandList->SetComputeRootDescriptorTable(setIndex, ptr);
            }
            else
            {
                mCommandList->SetGraphicsRootDescriptorTable(setIndex, ptr);
            }

            continue;
        }

        if (!instance)
        {
            continue;
        }

        // transition resources
        // TODO pass proper subresource (based on resource view that is written to resource binding)
        for (uint32 i = 0; i < instance->mResources.Size(); ++i)
        {
            const ResourceBindingInstance::Resource& resource = instance->mResources[i];

            Texture* textureResource = static_cast<Texture*>(resource.texture.Get());
            Buffer* bufferResource = static_cast<Buffer*>(resource.buffer.Get());

            if (textureResource || bufferResource)
            {
                switch (state.bindingLayout->mBindingSets[setIndex]->mBindings[i].resourceType)
                {
                case ShaderResourceType::CBuffer:
                    mResourceStateCache.EnsureResourceState(bufferResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
                    break;
                case ShaderResourceType::Texture:
                    // TODO limit shader visibility (based on binding layout properties)
                    mResourceStateCache.EnsureResourceState(textureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                    break;
                case ShaderResourceType::StructuredBuffer:
                    // TODO limit shader visibility (based on binding layout properties)
                    mResourceStateCache.EnsureResourceState(bufferResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                    break;
                case ShaderResourceType::WritableTexture:
                    mResourceStateCache.EnsureResourceState(textureResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                    break;
                case ShaderResourceType::WritableStructuredBuffer:
                    mResourceStateCache.EnsureResourceState(bufferResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                    break;
                }
            }
        }

        HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
        D3D12_GPU_DESCRIPTOR_HANDLE ptr = allocator.GetGpuHandle();
        ptr.ptr += (uint64)instance->mGpuDescriptorHeapOffset * allocator.GetDescriptorSize();

        if (pipelineType == PipelineType::Compute)
        {
            mCommandList->SetComputeRootDescriptorTable(setIndex, ptr);
        }
        else
        {
            mCommandList->SetGraphicsRootDescriptorTable(setIndex, ptr);
        }
    }

    state.bindingInstancesChanged = false;
    state.pendingDirectResourceBinds.Clear();
}

void CommandRecorder::Internal_UpdateComputeResourceBindingLayout()
{
    if (mComputeBindingState.bindingLayoutChanged)
    {
        mComputeBindingState.bindingLayoutChanged = false;

        mCommandList->SetComputeRootSignature(mComputeBindingState.bindingLayout->mRootSignature.Get());
    }
}

void CommandRecorder::Internal_PrepareForDraw()
{
    Internal_UpdateGraphicsPipelineState();
    Internal_UpdateGraphicsResourceBindingLayout();
    Internal_UpdateVetexAndIndexBuffers();
    Internal_UpdateResourceBindings(PipelineType::Graphics);

    NFE_ASSERT(mGraphicsPipelineState, "Graphics pipeline state not set");

    mResourceStateCache.FlushPendingBarriers(mCommandList);
}

void CommandRecorder::Internal_PrepareForDispatch()
{
    Internal_UpdateComputePipelineState();
    Internal_UpdateComputeResourceBindingLayout();
    Internal_UpdateResourceBindings(PipelineType::Compute);

    NFE_ASSERT(mComputePipelineState, "Compute pipeline state not set");

    mResourceStateCache.FlushPendingBarriers(mCommandList);
}

void CommandRecorder::Draw(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset)
{
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics, "Invalid queue type");

    Internal_PrepareForDraw();
    mCommandList->DrawInstanced(vertexNum, instancesNum, vertexOffset, instanceOffset);
}

void CommandRecorder::DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset)
{
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics, "Invalid queue type");

    Internal_PrepareForDraw();
    mCommandList->DrawIndexedInstanced(indexNum, instancesNum, indexOffset, vertexOffset, instanceOffset);
}

void CommandRecorder::SetComputePipelineState(const ComputePipelineStatePtr& state)
{
    ComputePipelineState* newState = static_cast<ComputePipelineState*>(state.Get());
    NFE_ASSERT(newState, "Invalid compute pipeline state");

    if (newState != mComputePipelineState)
    {
        Internal_GetReferencedResources().computePipelineStates.Insert(state);

        mComputePipelineState = newState;
        mComputePipelineStateChanged = true;
    }
}

void CommandRecorder::Dispatch(uint32 x, uint32 y, uint32 z)
{
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics || mQueueType == CommandQueueType::Compute, "Invalid queue type");

    Internal_PrepareForDispatch();

    mCommandList->Dispatch(x, y, z);
}

void CommandRecorder::DispatchIndirect(const BufferPtr& indirectArgBuffer, uint32 bufferOffset)
{
    NFE_ASSERT(mQueueType == CommandQueueType::Graphics || mQueueType == CommandQueueType::Compute, "Invalid queue type");

    const Buffer* buffer = static_cast<Buffer*>(indirectArgBuffer.Get());
    NFE_ASSERT(buffer, "Invalid buffer");

    Internal_GetReferencedResources().buffers.Insert(indirectArgBuffer);

    if (buffer->GetMode() != ResourceAccessMode::Upload && buffer->GetMode() != ResourceAccessMode::GPUOnly)
    {
        NFE_FATAL("Invalid access mode of indirect argument buffer");
    }

    const uint32 argumentsSize = sizeof(D3D12_DISPATCH_ARGUMENTS);
    NFE_ASSERT(bufferOffset + argumentsSize <= buffer->GetSize(), "Indirect argument buffer is too small");

    mResourceStateCache.EnsureResourceState(buffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    mResourceStateCache.FlushPendingBarriers(mCommandList);

    Internal_PrepareForDispatch();

    mCommandList->ExecuteIndirect(gDevice->GetIndirectDispatchCommandSignature(), 1, buffer->GetD3DResource(), bufferOffset, nullptr, 0);
}

void CommandRecorder::Internal_ReferenceBindingSetInstance(const ResourceBindingInstancePtr& bindingSetInstance)
{
    if (!bindingSetInstance)
    {
        return;
    }

    ReferencedResourcesList& referencedResources = Internal_GetReferencedResources();

    referencedResources.bindingSetInstances.Insert(bindingSetInstance);

    ResourceBindingInstance* instance = static_cast<ResourceBindingInstance*>(bindingSetInstance.Get());

    // add all resources used by this set as well
    for (uint32 i = 0; i < instance->mResources.Size(); ++i)
    {
        const ResourceBindingInstance::Resource& resource = instance->mResources[i];

        if (resource.texture)
        {
            referencedResources.textures.Insert(resource.texture);
        }

        if (resource.buffer)
        {
            referencedResources.buffers.Insert(resource.buffer);
        }
    }
}

void CommandRecorder::BeginDebugGroup(const char* text)
{
    NFE_UNUSED(text);
}

void CommandRecorder::EndDebugGroup()
{
}

void CommandRecorder::InsertDebugMarker(const char* text)
{
    NFE_UNUSED(text);
}

} // namespace Renderer
} // namespace NFE
