/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's command buffer
 */

// TODO:
// 1. Improve logging, but be careful - functions from this source file will be called thousands
//    times per frame. Too much messages could flood a logger output.

#include "PCH.hpp"
#include "CommandBuffer.hpp"
#include "RendererD3D11.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "RenderTarget.hpp"
#include "PipelineState.hpp"
#include "Sampler.hpp"
#include "Translations.hpp"

#include "nfCommon/Logger.hpp"
#include "nfCommon/Win/Common.hpp"  // required for ID3DUserDefinedAnnotation


namespace NFE {
namespace Renderer {

CommandBuffer::CommandBuffer(ID3D11DeviceContext* deviceContext)
    : mContext(deviceContext)
    , mCurrentPrimitiveType(PrimitiveType::Unknown)
    , mCurrentRenderTarget(nullptr)
    , mBindingLayout(nullptr)
    , mPipelineState(nullptr)
    , mCurrentPipelineState(nullptr)
    , mReset(false)
{
    HRESULT hr;
    hr = deviceContext->QueryInterface(IID_PPV_ARGS(&mUserDefinedAnnotation));
    if (FAILED(hr))
        mUserDefinedAnnotation.reset();

    Reset();
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::Reset()
{
    if (mReset)
        LOG_WARNING("Redundant command buffer reset");

    mReset = true;
    mStencilRef = mCurrentStencilRef = 0;
    mCurrentPrimitiveType = PrimitiveType::Unknown;
    mCurrentRenderTarget = nullptr;
    mBindingLayout = nullptr;
    mPipelineState = nullptr;
    mCurrentPipelineState = nullptr;

    mBoundVertexShader = nullptr;
    mBoundHullShader = nullptr;
    mBoundDomainShader = nullptr;
    mBoundGeometryShader = nullptr;
    mBoundPixelShader = nullptr;

    mContext->ClearState();
}

void CommandBuffer::SetViewport(float left, float width, float top, float height,
                                float minDepth, float maxDepth)
{
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = left;
    viewport.TopLeftY = top;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = minDepth;
    viewport.MaxDepth = maxDepth;

    mContext->RSSetViewports(1, &viewport);
}

void CommandBuffer::SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets)
{
    ID3D11Buffer* vbs[16];
    for (int i = 0; i < num; ++i)
    {
        Buffer* vertexBuffer = dynamic_cast<Buffer*>(vertexBuffers[i]);
        vbs[i] = vertexBuffer->mBuffer.get();
    }

    mContext->IASetVertexBuffers(0, num, vbs,
                                 reinterpret_cast<UINT*>(strides),
                                 reinterpret_cast<UINT*>(offsets));
}

void CommandBuffer::SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format)
{
    DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
    switch (format)
    {
        case IndexBufferFormat::Uint16:
            dxgiFormat = DXGI_FORMAT_R16_UINT;
            break;
        case IndexBufferFormat::Uint32:
            dxgiFormat = DXGI_FORMAT_R32_UINT;
            break;
    };

    Buffer* ib = dynamic_cast<Buffer*>(indexBuffer);
    mContext->IASetIndexBuffer(ib->mBuffer.get(), dxgiFormat, 0);
}

void CommandBuffer::BindResources(size_t slot, IResourceBindingInstance* bindingSetInstance)
{
    if (!mBindingLayout)
    {
        LOG_ERROR("Binding layout is not set");
        return;
    }

    const ResourceBindingInstance* instance =
        dynamic_cast<ResourceBindingInstance*>(bindingSetInstance);

    if (slot >= mBindingLayout->mBindingSets.size())
    {
        LOG_ERROR("Invalid binding set slot");
        return;
    }

    const ResourceBindingSet* bindingSet = mBindingLayout->mBindingSets[slot];
    for (size_t i = 0; i < bindingSet->mBindings.size(); ++i)
    {
        const ResourceBindingDesc& bindingDesc = bindingSet->mBindings[i];
        UINT slotOffset = bindingDesc.slot;

        if (bindingDesc.resourceType == ShaderResourceType::CBuffer)
        {
            ID3D11Buffer* buffer = instance ?
                static_cast<ID3D11Buffer*>(instance->mViews[i]) : nullptr;

            switch (bindingSet->mShaderVisibility)
            {
            case ShaderType::Vertex:
                mContext->VSSetConstantBuffers(slotOffset, 1, &buffer);
                break;
            case ShaderType::Domain:
                mContext->DSSetConstantBuffers(slotOffset, 1, &buffer);
                break;
            case ShaderType::Hull:
                mContext->HSSetConstantBuffers(slotOffset, 1, &buffer);
                break;
            case ShaderType::Geometry:
                mContext->GSSetConstantBuffers(slotOffset, 1, &buffer);
                break;
            case ShaderType::Pixel:
                mContext->PSSetConstantBuffers(slotOffset, 1, &buffer);
                break;
            case ShaderType::All:
                mContext->VSSetConstantBuffers(slotOffset, 1, &buffer);
                mContext->DSSetConstantBuffers(slotOffset, 1, &buffer);
                mContext->HSSetConstantBuffers(slotOffset, 1, &buffer);
                mContext->GSSetConstantBuffers(slotOffset, 1, &buffer);
                mContext->PSSetConstantBuffers(slotOffset, 1, &buffer);
                break;
            }
        }
        else if (bindingDesc.resourceType == ShaderResourceType::Texture)
        {
            ID3D11ShaderResourceView* srv =
                instance ? static_cast<ID3D11ShaderResourceView*>(instance->mViews[i]) : nullptr;

            switch (bindingSet->mShaderVisibility)
            {
            case ShaderType::Vertex:
                mContext->VSSetShaderResources(slotOffset, 1, &srv);
                break;
            case ShaderType::Domain:
                mContext->DSSetShaderResources(slotOffset, 1, &srv);
                break;
            case ShaderType::Hull:
                mContext->HSSetShaderResources(slotOffset, 1, &srv);
                break;
            case ShaderType::Geometry:
                mContext->GSSetShaderResources(slotOffset, 1, &srv);
                break;
            case ShaderType::Pixel:
                mContext->PSSetShaderResources(slotOffset, 1, &srv);
                break;
            case ShaderType::All:
                mContext->VSSetShaderResources(slotOffset, 1, &srv);
                mContext->DSSetShaderResources(slotOffset, 1, &srv);
                mContext->HSSetShaderResources(slotOffset, 1, &srv);
                mContext->GSSetShaderResources(slotOffset, 1, &srv);
                mContext->PSSetShaderResources(slotOffset, 1, &srv);
                break;
            };
        }
    }
}

void CommandBuffer::BindDynamicBuffer(size_t slot, IBuffer* buffer)
{
    if (!mBindingLayout)
    {
        LOG_ERROR("Binding layout is not set");
        return;
    }

    if (slot >= mBindingLayout->mDynamicBuffers.size())
    {
        LOG_ERROR("Invalid dynamic buffer slot");
        return;
    }

    Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer);
    if (!bufferPtr)
    {
        LOG_ERROR("Invalid buffer");
        return;
    }

    const ShaderType targetShader = mBindingLayout->mDynamicBuffers[slot].shaderVisibility;
    UINT slotOffset = mBindingLayout->mDynamicBuffers[slot].slot;

    ID3D11Buffer* d3dBuffer = bufferPtr->mBuffer.get();

    switch (targetShader)
    {
    case ShaderType::Vertex:
        mContext->VSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
        break;
    case ShaderType::Domain:
        mContext->DSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
        break;
    case ShaderType::Hull:
        mContext->HSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
        break;
    case ShaderType::Geometry:
        mContext->GSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
        break;
    case ShaderType::Pixel:
        mContext->PSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
        break;
    case ShaderType::All:
        mContext->VSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
        mContext->DSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
        mContext->HSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
        mContext->GSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
        mContext->PSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
        break;
    }
}

void CommandBuffer::UpdateSamplers()
{
    for (size_t j = 0; j < mBindingLayout->mBindingSets.size(); ++j)
    {
        const ResourceBindingSet* bindingSet = mBindingLayout->mBindingSets[j];

        for (size_t i = 0; i < bindingSet->mBindings.size(); ++i)
        {
            const ResourceBindingDesc& bindingDesc = bindingSet->mBindings[i];
            UINT slotOffset = bindingDesc.slot;

            if (bindingDesc.resourceType != ShaderResourceType::Texture)
                continue;

            Sampler* sampler = dynamic_cast<Sampler*>(bindingDesc.staticSampler);
            if (!sampler)
                continue;

            ID3D11SamplerState* samplerState = sampler->mSamplerState.get();

            switch (bindingSet->mShaderVisibility)
            {
            case ShaderType::Vertex:
                mContext->VSSetSamplers(slotOffset, 1, &samplerState);
                break;
            case ShaderType::Domain:
                mContext->DSSetSamplers(slotOffset, 1, &samplerState);
                break;
            case ShaderType::Hull:
                mContext->HSSetSamplers(slotOffset, 1, &samplerState);
                break;
            case ShaderType::Geometry:
                mContext->GSSetSamplers(slotOffset, 1, &samplerState);
                break;
            case ShaderType::Pixel:
                mContext->PSSetSamplers(slotOffset, 1, &samplerState);
                break;
            case ShaderType::All:
                mContext->VSSetSamplers(slotOffset, 1, &samplerState);
                mContext->DSSetSamplers(slotOffset, 1, &samplerState);
                mContext->HSSetSamplers(slotOffset, 1, &samplerState);
                mContext->GSSetSamplers(slotOffset, 1, &samplerState);
                mContext->PSSetSamplers(slotOffset, 1, &samplerState);
                break;
            };
        }
    }
}

void CommandBuffer::SetRenderTarget(IRenderTarget* renderTarget)
{
    RenderTarget* rt = dynamic_cast<RenderTarget*>(renderTarget);
    if (rt == nullptr && renderTarget != nullptr)
        LOG_ERROR("Invalid 'renderTarget' pointer");

    if (rt == mCurrentRenderTarget)
        return;

    // disable rendertargets
    if (rt == nullptr)
    {
        mContext->OMSetRenderTargets(0, NULL, NULL);
        mCurrentRenderTarget = rt;
        return;
    }

    ID3D11DepthStencilView* dsv = nullptr;
    ID3D11RenderTargetView* rtvs[MAX_RENDER_TARGETS] = { nullptr };
    size_t num = rt->mRTVs.size();

    for (size_t i = 0; i < num; ++i)
        rtvs[i] = rt->mRTVs[i].get();

    if (rt->mDepthBuffer)
        dsv = rt->mDepthBuffer->mDSV.get();

    mContext->OMSetRenderTargets(static_cast<UINT>(num), rtvs, dsv);
    mCurrentRenderTarget = rt;
}

void CommandBuffer::SetResourceBindingLayout(IResourceBindingLayout* layout)
{
    mBindingLayout = dynamic_cast<ResourceBindingLayout*>(layout);
}

void CommandBuffer::SetPipelineState(IPipelineState* state)
{
    mPipelineState = dynamic_cast<PipelineState*>(state);
    NFE_ASSERT(mPipelineState != nullptr, "Invalid pipeline state");

    mBindingLayout = mPipelineState->mResBindingLayout;

    if (mPipelineState->mVertexShader != mBoundVertexShader)
    {
        mContext->VSSetShader(mPipelineState->mVertexShader, nullptr, 0);
        mBoundVertexShader = mPipelineState->mVertexShader;
    }

    if (mPipelineState->mGeometryShader != mBoundGeometryShader)
    {
        mContext->GSSetShader(mPipelineState->mGeometryShader, nullptr, 0);
        mBoundGeometryShader = mPipelineState->mGeometryShader;
    }

    if (mPipelineState->mHullShader != mBoundHullShader)
    {
        mContext->HSSetShader(mPipelineState->mHullShader, nullptr, 0);
        mBoundHullShader = mPipelineState->mHullShader;
    }

    if (mPipelineState->mDomainShader != mBoundDomainShader)
    {
        mContext->DSSetShader(mPipelineState->mDomainShader, nullptr, 0);
        mBoundDomainShader = mPipelineState->mDomainShader;
    }

    if (mPipelineState->mPixelShader != mBoundPixelShader)
    {
        mContext->PSSetShader(mPipelineState->mPixelShader, nullptr, 0);
        mBoundPixelShader = mPipelineState->mPixelShader;
    }
}

void CommandBuffer::SetStencilRef(unsigned char ref)
{
    mStencilRef = ref;
}

void CommandBuffer::SetScissors(int left, int top, int right, int bottom)
{
    D3D11_RECT rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    mContext->RSSetScissorRects(1, &rect);
}

void* CommandBuffer::MapBuffer(IBuffer* buffer, MapType type)
{
    Buffer* buf = dynamic_cast<Buffer*>(buffer);
    if (!buf)
        return nullptr;

    D3D11_MAP mapType;
    switch (type)
    {
    case MapType::ReadOnly:
        mapType = D3D11_MAP_READ;
        break;
    case MapType::WriteOnly:
        mapType = D3D11_MAP_WRITE_DISCARD;
        break;
    case MapType::ReadWrite:
        mapType = D3D11_MAP_READ_WRITE;
        break;
    default:
        return nullptr;
    }

    D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
    ID3D11Resource* res = reinterpret_cast<ID3D11Resource*>(buf->mBuffer.get());
    HRESULT hr = D3D_CALL_CHECK(mContext->Map(res, 0, mapType, 0, &mapped));
    if (FAILED(hr))
        return nullptr;

    return mapped.pData;
}

void CommandBuffer::UnmapBuffer(IBuffer* buffer)
{
    Buffer* buf = dynamic_cast<Buffer*>(buffer);
    if (!buf)
        return;

    ID3D11Resource* res = reinterpret_cast<ID3D11Resource*>(buf->mBuffer.get());
    mContext->Unmap(res, 0);
}

bool CommandBuffer::WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data)
{
    Buffer* buf = dynamic_cast<Buffer*>(buffer);
    if (!buf)
        return false;

    D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
    ID3D11Resource* res = reinterpret_cast<ID3D11Resource*>(buf->mBuffer.get());
    HRESULT hr = D3D_CALL_CHECK(mContext->Map(res, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    if (FAILED(hr))
        return false;

    memcpy((char*)mapped.pData + offset, data, size);
    mContext->Unmap(res, 0);
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

    Texture* destTex = dynamic_cast<Texture*>(dest);
    if (destTex == nullptr)
    {
        LOG_ERROR("Invalid 'dest' pointer");
        return;
    }

    mContext->CopyResource(reinterpret_cast<ID3D11Resource*>(destTex->mTextureGeneric),
                           reinterpret_cast<ID3D11Resource*>(srcTex->mTextureGeneric));
}

void CommandBuffer::Clear(int flags, uint32 numTargets, const uint32* slots,
                          const Math::Float4* colors, float depthValue, uint8 stencilValue)
{
    if (mCurrentRenderTarget)
    {
        if (flags & ClearFlagsColor)
        {
            for (uint32 i = 0; i < numTargets; ++i)
            {
                uint32 slot = i;
                if (slots)
                {
                    if (slots[i] >= mCurrentRenderTarget->mRTVs.size())
                    {
                        LOG_ERROR("Invalid render target texture slot = %u", slots[i]);
                        return;
                    }

                    slot = slots[i];
                }

                mContext->ClearRenderTargetView(mCurrentRenderTarget->mRTVs[slot].get(),
                                                reinterpret_cast<const float*>(&colors[i]));
            }
        }

        if (flags & (ClearFlagsDepth | ClearFlagsStencil))
        {
            ID3D11DepthStencilView* dsv;
            if (mCurrentRenderTarget->mDepthBuffer)
            {
                dsv = mCurrentRenderTarget->mDepthBuffer->mDSV.get();
                if (!dsv)
                    return;

                UINT d3dFlags = 0;
                if (flags & ClearFlagsDepth)      d3dFlags = D3D11_CLEAR_DEPTH;
                if (flags & ClearFlagsStencil)    d3dFlags |= D3D11_CLEAR_STENCIL;
                mContext->ClearDepthStencilView(dsv, d3dFlags, depthValue, stencilValue);
            }
        }
    }
}

void CommandBuffer::UpdateState()
{
    if (mCurrentPipelineState != mPipelineState || mCurrentStencilRef != mStencilRef)
    {
        if (mBindingLayout != mPipelineState->mResBindingLayout)
            LOG_ERROR("Binding layout mismatch");

        UpdateSamplers();

        mContext->OMSetBlendState(mPipelineState->mBS.get(), nullptr, 0xFFFFFFFF);
        mContext->RSSetState(mPipelineState->mRS.get());
        mContext->OMSetDepthStencilState(mPipelineState->mDS.get(), mStencilRef);
        mContext->IASetInputLayout(mPipelineState->mVertexLayout->mIL.get());

        mCurrentPipelineState = mPipelineState;
        mCurrentStencilRef = mStencilRef;

        if (mPipelineState->mPrimitiveType != mCurrentPrimitiveType)
        {
            mCurrentPrimitiveType = mPipelineState->mPrimitiveType;
            D3D11_PRIMITIVE_TOPOLOGY topology = TranslatePrimitiveType(mCurrentPrimitiveType,
                                                                       mPipelineState->mNumControlPoints);
            mContext->IASetPrimitiveTopology(topology);
        }
    }
}

void CommandBuffer::Draw(int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    UpdateState();

    if (instancesNum >= 0)
        mContext->DrawInstanced(vertexNum, instancesNum, vertexOffset, instanceOffset);
    else
        mContext->Draw(vertexNum, vertexOffset);
}

void CommandBuffer::DrawIndexed(int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    UpdateState();

    if (instancesNum >= 0)
        mContext->DrawIndexedInstanced(indexNum, instancesNum, indexOffset, vertexOffset,
                                       instanceOffset);
    else
        mContext->DrawIndexed(indexNum, indexOffset, vertexOffset);
}

std::unique_ptr<ICommandList> CommandBuffer::Finish()
{
    if (!mReset)
    {
        LOG_ERROR("Command buffer is not in recording state");
        return nullptr;
    }

    mReset = false;

    HRESULT hr;
    ID3D11CommandList* d3dList;
    hr = D3D_CALL_CHECK(mContext->FinishCommandList(FALSE, &d3dList));
    if (FAILED(hr))
        return nullptr;

    // TODO: use memory pool
    std::unique_ptr<CommandList> list(new (std::nothrow) CommandList);
    if (!list)
    {
        D3D_SAFE_RELEASE(d3dList);
        LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    list->mD3DList = d3dList;
    return list;
}

void CommandBuffer::BeginDebugGroup(const char* text)
{
    if (mUserDefinedAnnotation.get())
    {
        std::wstring wideText;
        if (Common::UTF8ToUTF16(text, wideText))
            mUserDefinedAnnotation->BeginEvent(wideText.c_str());
        else
        {
            // We must begin an event, even if the UTF-8 -> UTF-16 conversion fails,
            // because user will call EndDebugGroup().
            mUserDefinedAnnotation->BeginEvent(L"");
        }
    }
}

void CommandBuffer::EndDebugGroup()
{
    if (mUserDefinedAnnotation.get())
        mUserDefinedAnnotation->EndEvent();
}

void CommandBuffer::InsertDebugMarker(const char* text)
{
    if (mUserDefinedAnnotation.get())
    {
        std::wstring wideText;
        if (Common::UTF8ToUTF16(text, wideText))
            mUserDefinedAnnotation->SetMarker(wideText.c_str());
    }
}

} // namespace Renderer
} // namespace NFE
