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

CommandBuffer::CommandBuffer(ID3D12Device* device)
    : mCurrRenderTarget(nullptr)
    , mBindingLayout(nullptr)
    , mCurrShaderProgram(nullptr)
    , mShaderProgram(nullptr)
    , mCurrPipelineState(nullptr)
    , mPipelineState(nullptr)
    , mCurrPrimitiveType(PrimitiveType::Unknown)
    , mPrimitiveType(PrimitiveType::Unknown)
{
    HRESULT hr;

    hr = D3D_CALL_CHECK(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                       IID_PPV_ARGS(&mCommandAllocator)));
    if (FAILED(hr))
        return;

    hr = D3D_CALL_CHECK(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                  mCommandAllocator.get(), nullptr,
                                                  IID_PPV_ARGS(&mCommandList)));
    if (FAILED(hr))
        return;

    // we don't want the command list to be in recording state
    hr = D3D_CALL_CHECK(mCommandList->Close());
    if (FAILED(hr))
        return;
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::Reset()
{
    HRESULT hr;

    hr = D3D_CALL_CHECK(mCommandAllocator->Reset());
    if (FAILED(hr))
        return;

    hr = D3D_CALL_CHECK(mCommandList->Reset(mCommandAllocator.get(), nullptr));
    if (FAILED(hr))
        return;

    ID3D12DescriptorHeap* heaps[] = { gDevice->mCbvSrvUavHeap.get() };
    mCommandList->SetDescriptorHeaps(1, heaps);

    mCurrRenderTarget = nullptr;
    mBindingLayout = nullptr;
    mCurrShaderProgram = nullptr;
    mShaderProgram = nullptr;
    mCurrPipelineState = nullptr;
    mPipelineState = nullptr;
    mCurrPrimitiveType = mPrimitiveType = PrimitiveType::Unknown;
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

    D3D12_GPU_DESCRIPTOR_HANDLE ptr = gDevice->mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
    ptr.ptr += instance->mDescriptorHeapOffset * gDevice->mCbvSrvUavDescSize;
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
        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[MAX_RENDER_TARGETS];

        for (size_t i = 0; i < mCurrRenderTarget->mTextures.size(); ++i)
        {
            Texture* tex = mCurrRenderTarget->mTextures[i];
            int currBuffer = tex->mCurrentBuffer;

            rtvs[i] = mCurrRenderTarget->mRTVs[currBuffer][i];

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
    mCommandList->SetGraphicsRootSignature(mBindingLayout->mRootSignature.get());
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
        for (size_t i = 0; i < mCurrRenderTarget->mTextures.size(); ++i)
        {
            Texture* tex = mCurrRenderTarget->mTextures[i];
            int currentBuffer = tex->mCurrentBuffer;

            mCommandList->ClearRenderTargetView(mCurrRenderTarget->mRTVs[currentBuffer][i], color,
                                                0, nullptr);
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

        FullPipelineStateParts parts(mPipelineState, mShaderProgram);
        FullPipelineState* fullPipelineState = gDevice->GetFullPipelineState(parts);
        mCommandList->SetPipelineState(fullPipelineState->mPipelineState.get());
        mCommandList->SetGraphicsRootSignature(mBindingLayout->mRootSignature.get());

        mCurrPipelineState = mPipelineState;
        mCurrShaderProgram = mShaderProgram;
    }

    if (mPrimitiveType != mCurrPrimitiveType)
    {
        mCurrPrimitiveType = mPrimitiveType;
        D3D12_PRIMITIVE_TOPOLOGY topology = TranslatePrimitiveType(mPrimitiveType);
        mCommandList->IASetPrimitiveTopology(topology);
    }
}

void CommandBuffer::Draw(PrimitiveType type, int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    mPrimitiveType = type;
    UpdateStates();
    mCommandList->DrawInstanced(vertexNum, instancesNum, vertexOffset, instanceOffset);
}

void CommandBuffer::DrawIndexed(PrimitiveType type, int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    mPrimitiveType = type;
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
