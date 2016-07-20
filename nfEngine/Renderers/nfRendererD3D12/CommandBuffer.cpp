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
    , mFrameCount(3)
    , mCurrFrame(0)
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
                                                  mCommandAllocators[mCurrFrame].get(), nullptr,
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

CommandBuffer::~CommandBuffer()
{
    ::CloseHandle(mFenceEvent);
}

void CommandBuffer::Reset()
{
    HRESULT hr;

    hr = D3D_CALL_CHECK(mCommandAllocators[mCurrFrame]->Reset());
    if (FAILED(hr))
        return;

    hr = D3D_CALL_CHECK(mCommandList->Reset(mCommandAllocators[mCurrFrame].get(), nullptr));
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

    HeapAllocator& allocator = gDevice->GetCbvSrvUavHeapAllocator();
    D3D12_GPU_DESCRIPTOR_HANDLE ptr = allocator.GetGpuHandle();
    ptr.ptr += instance->mDescriptorHeapOffset * allocator.GetDescriptorSize();
    mCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(slot), ptr);
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
        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[MAX_RENDER_TARGETS];

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

        mCommandList->OMSetRenderTargets(static_cast<UINT>(mCurrRenderTarget->mTextures.size()),
                                         rtvs, FALSE, nullptr);
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

            if (tex->mClass != Texture::Class::Backbuffer)
                continue;

            // make transition to "Present" state if the render target's texture is backbuffer
            if (tex->mResourceState != D3D12_RESOURCE_STATE_PRESENT)
            {
                D3D12_RESOURCE_BARRIER rb;
                ZeroMemory(&rb, sizeof(rb));
                rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                rb.Transition.pResource = tex->mBuffers[currBuffer].get();
                rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                rb.Transition.StateBefore = tex->mResourceState;
                rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                mCommandList->ResourceBarrier(1, &rb);

                tex->mResourceState = D3D12_RESOURCE_STATE_PRESENT;
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
    UNUSED(ref);
}


bool CommandBuffer::WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data)
{
    Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer);
    if (!bufferPtr || !bufferPtr->mData)
    {
        LOG_ERROR("Invalid buffer");
        return false;
    }

    if (offset + size > bufferPtr->mSize)
        return false;

    char* target = reinterpret_cast<char*>(bufferPtr->mData);
    memcpy(target + offset, data, size);
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
    if (mCurrRenderTarget != nullptr)
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

    UNUSED(flags);
    UNUSED(depthValue);
    UNUSED(stencilValue);
}

void CommandBuffer::UpdateStates()
{
    if (mCurrPipelineState != mPipelineState || mCurrShaderProgram != mShaderProgram)
    {
        if (mBindingLayout != mPipelineState->mBindingLayout)
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
    uint64 currFenceValue = mFenceValues[mCurrFrame];

    HRESULT hr = D3D_CALL_CHECK(commandQueue->Signal(mFence.get(), currFenceValue));
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to enqueue fence value update");
        return false;
    }

    // update frame index
    mCurrFrame++;
    if (mCurrFrame >= mFrameCount)
        mCurrFrame = 0;

    // wait for frame
    if (mFence->GetCompletedValue() < mFenceValues[mCurrFrame])
    {
        hr = D3D_CALL_CHECK(mFence->SetEventOnCompletion(mFenceValues[mCurrFrame], mFenceEvent));
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

    mFenceValues[mCurrFrame] = currFenceValue + 1;
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
