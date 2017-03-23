/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's command buffer
 */

// TODO:
// 1. Improve logging, but be careful - functions from this source file will be called thousands
//    times per frame. Too much messages could flood a logger output.

#include "PCH.hpp"
#include "CommandRecorder.hpp"
#include "RendererD3D11.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "RenderTarget.hpp"
#include "ResourceBinding.hpp"
#include "PipelineState.hpp"
#include "ComputePipelineState.hpp"
#include "Sampler.hpp"
#include "Translations.hpp"

#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/System/Assertion.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/System/Win/Common.hpp"  // required for ID3DUserDefinedAnnotation


namespace NFE {
namespace Renderer {

CommandRecorder::CommandRecorder(ID3D11DeviceContext* deviceContext)
    : mContext(deviceContext)
    , mCurrentPrimitiveType(PrimitiveType::Unknown)
    , mCurrentRenderTarget(nullptr)
    , mBindingLayout(nullptr)
    , mPipelineState(nullptr)
    , mCurrentPipelineState(nullptr)
    , mReset(false)
    , mBoundVertexShader(nullptr)
    , mBoundHullShader(nullptr)
    , mBoundDomainShader(nullptr)
    , mBoundGeometryShader(nullptr)
    , mBoundPixelShader(nullptr)
    , mBoundComputeShader(nullptr)
    , mComputeBindingLayout(nullptr)
    , mComputePipelineState(nullptr)
{
    HRESULT hr;
    hr = deviceContext->QueryInterface(IID_PPV_ARGS(&mUserDefinedAnnotation));
    if (FAILED(hr))
        mUserDefinedAnnotation.reset();
}

CommandRecorder::~CommandRecorder()
{
}

bool CommandRecorder::Begin()
{
    if (mReset)
    {
        LOG_ERROR("CommandRecorder is already in recording state");
        return false;
    }

    mReset = true;
    mStencilRef = mCurrentStencilRef = 0;
    mCurrentPrimitiveType = PrimitiveType::Unknown;
    mCurrentRenderTarget = nullptr;
    mBindingLayout = nullptr;
    mPipelineState = nullptr;
    mCurrentPipelineState = nullptr;
    mComputeBindingLayout = nullptr;
    mComputePipelineState = nullptr;

    mBoundVertexShader = nullptr;
    mBoundHullShader = nullptr;
    mBoundDomainShader = nullptr;
    mBoundGeometryShader = nullptr;
    mBoundPixelShader = nullptr;
    mBoundComputeShader = nullptr;

    mContext->ClearState();

    return true;
}

void CommandRecorder::SetViewport(float left, float width, float top, float height,
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

void CommandRecorder::SetVertexBuffers(int num, const BufferPtr* vertexBuffers, int* strides, int* offsets)
{
    ID3D11Buffer* vbs[16];
    for (int i = 0; i < num; ++i)
    {
        const Buffer* vertexBuffer = dynamic_cast<Buffer*>(vertexBuffers[i].get());
        vbs[i] = vertexBuffer->mBuffer.get();
    }

    mContext->IASetVertexBuffers(0, num, vbs,
                                 reinterpret_cast<UINT*>(strides),
                                 reinterpret_cast<UINT*>(offsets));
}

void CommandRecorder::SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format)
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

    const Buffer* ib = dynamic_cast<Buffer*>(indexBuffer.get());
    mContext->IASetIndexBuffer(ib->mBuffer.get(), dxgiFormat, 0);
}

void CommandRecorder::BindResources(size_t slot, const ResourceBindingInstancePtr& bindingSetInstance)
{
    if (!mBindingLayout)
    {
        LOG_ERROR("Binding layout is not set");
        return;
    }

    const ResourceBindingInstance* instance =
        dynamic_cast<ResourceBindingInstance*>(bindingSetInstance.get());

    if (slot >= mBindingLayout->mBindingSets.size())
    {
        LOG_ERROR("Invalid binding set slot");
        return;
    }

    const ResourceBindingSet* bindingSet = mBindingLayout->mBindingSets[slot].get();
    for (size_t i = 0; i < bindingSet->mBindings.size(); ++i)
    {
        const ResourceBindingDesc& bindingDesc = bindingSet->mBindings[i];
        UINT slotOffset = bindingDesc.slot;

        if (bindingDesc.resourceType == ShaderResourceType::CBuffer)
        {
            ID3D11Buffer* buffer = instance ?
                static_cast<ID3D11Buffer*>(instance->mCBuffers[i]) : nullptr;

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
                instance ? static_cast<ID3D11ShaderResourceView*>(instance->mViews[i].get()) : nullptr;

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

void CommandRecorder::BindVolatileCBuffer(size_t slot, const BufferPtr& buffer)
{
    if (!mBindingLayout)
    {
        LOG_ERROR("Binding layout is not set");
        return;
    }

    if (slot >= mBindingLayout->mVolatileCBuffers.size())
    {
        LOG_ERROR("Invalid dynamic buffer slot");
        return;
    }

    const Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer.get());
    if (!bufferPtr)
    {
        LOG_ERROR("Invalid buffer");
        return;
    }

    const ShaderType targetShader = mBindingLayout->mVolatileCBuffers[slot].shaderVisibility;
    UINT slotOffset = mBindingLayout->mVolatileCBuffers[slot].slot;

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

void CommandRecorder::UpdateSamplers()
{
    for (size_t j = 0; j < mBindingLayout->mBindingSets.size(); ++j)
    {
        const ResourceBindingSet* bindingSet = mBindingLayout->mBindingSets[j].get();

        for (size_t i = 0; i < bindingSet->mBindings.size(); ++i)
        {
            const ResourceBindingDesc& bindingDesc = bindingSet->mBindings[i];
            UINT slotOffset = bindingDesc.slot;

            if (bindingDesc.resourceType != ShaderResourceType::Texture)
                continue;

            Sampler* sampler = dynamic_cast<Sampler*>(bindingDesc.staticSampler.get());
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

void CommandRecorder::SetRenderTarget(const RenderTargetPtr& renderTarget)
{
    RenderTarget* rt = dynamic_cast<RenderTarget*>(renderTarget.get());
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

void CommandRecorder::SetResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    mBindingLayout = dynamic_cast<ResourceBindingLayout*>(layout.get());
}

void CommandRecorder::SetPipelineState(const PipelineStatePtr& state)
{
    mPipelineState = dynamic_cast<PipelineState*>(state.get());
    NFE_ASSERT(mPipelineState != nullptr, "Invalid pipeline state");

    mBindingLayout = dynamic_cast<ResourceBindingLayout*>(mPipelineState->mResBindingLayout.get());

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

void CommandRecorder::SetStencilRef(unsigned char ref)
{
    mStencilRef = ref;
}

void CommandRecorder::SetScissors(int left, int top, int right, int bottom)
{
    D3D11_RECT rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    mContext->RSSetScissorRects(1, &rect);
}

void* CommandRecorder::MapBuffer(const BufferPtr& buffer, MapType type)
{
    NFE_ASSERT(buffer, "Invalid buffer");
    Buffer* buf = dynamic_cast<Buffer*>(buffer.get());
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

void CommandRecorder::UnmapBuffer(const BufferPtr& buffer)
{
    NFE_ASSERT(buffer, "Invalid buffer");
    const Buffer* buf = dynamic_cast<Buffer*>(buffer.get());
    if (!buf)
        return;

    ID3D11Resource* res = reinterpret_cast<ID3D11Resource*>(buf->mBuffer.get());
    mContext->Unmap(res, 0);
}

bool CommandRecorder::WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data)
{
    NFE_ASSERT(buffer, "Invalid buffer");
    const Buffer* buf = dynamic_cast<Buffer*>(buffer.get());
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

void CommandRecorder::CopyTexture(const TexturePtr& src, const TexturePtr& dest)
{
    NFE_ASSERT(src, "Invalid source texture");
    NFE_ASSERT(dest, "Invalid destination texture");

    Texture* srcTex = dynamic_cast<Texture*>(src.get());
    if (srcTex == nullptr)
    {
        LOG_ERROR("Invalid 'src' pointer");
        return;
    }

    Texture* destTex = dynamic_cast<Texture*>(dest.get());
    if (destTex == nullptr)
    {
        LOG_ERROR("Invalid 'dest' pointer");
        return;
    }

    mContext->CopyResource(reinterpret_cast<ID3D11Resource*>(destTex->mTextureGeneric),
                           reinterpret_cast<ID3D11Resource*>(srcTex->mTextureGeneric));
}

void CommandRecorder::Clear(int flags, uint32 numTargets, const uint32* slots,
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

void CommandRecorder::UpdateState()
{
    if (mCurrentPipelineState != mPipelineState || mCurrentStencilRef != mStencilRef)
    {
        if (mBindingLayout != mPipelineState->mResBindingLayout.get())
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

void CommandRecorder::Draw(int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    UpdateState();

    if (instancesNum >= 0)
        mContext->DrawInstanced(vertexNum, instancesNum, vertexOffset, instanceOffset);
    else
        mContext->Draw(vertexNum, vertexOffset);
}

void CommandRecorder::DrawIndexed(int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    UpdateState();

    if (instancesNum >= 0)
        mContext->DrawIndexedInstanced(indexNum, instancesNum, indexOffset, vertexOffset,
                                       instanceOffset);
    else
        mContext->DrawIndexed(indexNum, indexOffset, vertexOffset);
}

void CommandRecorder::BindComputeResources(size_t slot, const ResourceBindingInstancePtr& bindingSetInstance)
{
    if (!mComputeBindingLayout)
    {
        LOG_ERROR("Binding layout is not set");
        return;
    }

    const ResourceBindingInstance* instance = dynamic_cast<ResourceBindingInstance*>(bindingSetInstance.get());
    if (slot >= mComputeBindingLayout->mBindingSets.size())
    {
        LOG_ERROR("Invalid binding set slot");
        return;
    }

    const ResourceBindingSet* bindingSet = mComputeBindingLayout->mBindingSets[slot].get();
    for (size_t i = 0; i < bindingSet->mBindings.size(); ++i)
    {
        const ResourceBindingDesc& bindingDesc = bindingSet->mBindings[i];
        UINT slotOffset = bindingDesc.slot;

        switch (bindingDesc.resourceType)
        {
            case ShaderResourceType::CBuffer:
            {
                ID3D11Buffer* buffer = instance ? static_cast<ID3D11Buffer*>(instance->mCBuffers[i]) : nullptr;
                mContext->CSSetConstantBuffers(slotOffset, 1, &buffer);
                break;
            }

            case ShaderResourceType::Texture:
            {
                ID3D11ShaderResourceView* srv =
                    instance ? static_cast<ID3D11ShaderResourceView*>(instance->mViews[i].get()) : nullptr;
                mContext->CSSetShaderResources(slotOffset, 1, &srv);
                break;
            }

            case ShaderResourceType::WritableTexture:
            {
                ID3D11UnorderedAccessView* uav =
                    instance ? static_cast<ID3D11UnorderedAccessView*>(instance->mViews[i].get()) : nullptr;
                mContext->CSSetUnorderedAccessViews(slotOffset, 1, &uav, nullptr);
                break;
            }
        }
    }
}

void CommandRecorder::BindComputeVolatileCBuffer(size_t slot, const BufferPtr& buffer)
{
    if (!mComputeBindingLayout)
    {
        LOG_ERROR("Binding layout is not set");
        return;
    }

    if (slot >= mComputeBindingLayout->mVolatileCBuffers.size())
    {
        LOG_ERROR("Invalid dynamic buffer slot");
        return;
    }

    const Buffer* bufferPtr = dynamic_cast<Buffer*>(buffer.get());
    if (!bufferPtr)
    {
        LOG_ERROR("Invalid buffer");
        return;
    }

    UINT slotOffset = mComputeBindingLayout->mVolatileCBuffers[slot].slot;
    ID3D11Buffer* d3dBuffer = bufferPtr->mBuffer.get();
    mContext->CSSetConstantBuffers(slotOffset, 1, &d3dBuffer);
}

void CommandRecorder::SetComputeResourceBindingLayout(const ResourceBindingLayoutPtr& layout)
{
    mComputeBindingLayout = dynamic_cast<ResourceBindingLayout*>(layout.get());
}

void CommandRecorder::SetComputePipelineState(const ComputePipelineStatePtr& state)
{
    mComputePipelineState = dynamic_cast<ComputePipelineState*>(state.get());
    NFE_ASSERT(mComputePipelineState != nullptr, "Invalid compute pipeline state");

    if (mComputePipelineState->GetShader() != mBoundComputeShader)
    {
        mBoundComputeShader = mComputePipelineState->GetShader();
        mContext->CSSetShader(mBoundComputeShader, nullptr, 0);
    }
}

void CommandRecorder::Dispatch(uint32 x, uint32 y, uint32 z)
{
    if (mComputeBindingLayout != mComputePipelineState->GetResBindingLayout())
    {
        LOG_ERROR("Binding layout mismatch");
        return;
    }

    mContext->Dispatch(x, y, z);
}

CommandListID CommandRecorder::Finish()
{
    if (!mReset)
    {
        LOG_ERROR("Command buffer is not in recording state");
        return 0;
    }

    mReset = false;

    ID3D11CommandList* d3dList;
    HRESULT hr = D3D_CALL_CHECK(mContext->FinishCommandList(FALSE, &d3dList));
    if (FAILED(hr))
        return 0;

    return gDevice->RegisterCommandList(d3dList);
}

void CommandRecorder::BeginDebugGroup(const char* text)
{
    if (mUserDefinedAnnotation.get())
    {
        std::wstring wideText;
        if (Common::UTF8ToUTF16(text, wideText))
            mUserDefinedAnnotation->BeginEvent(wideText.Str());
        else
        {
            // We must begin an event, even if the UTF-8 -> UTF-16 conversion fails,
            // because user will call EndDebugGroup().
            mUserDefinedAnnotation->BeginEvent(L"");
        }
    }
}

void CommandRecorder::EndDebugGroup()
{
    if (mUserDefinedAnnotation.get())
        mUserDefinedAnnotation->EndEvent();
}

void CommandRecorder::InsertDebugMarker(const char* text)
{
    if (mUserDefinedAnnotation.get())
    {
        std::wstring wideText;
        if (Common::UTF8ToUTF16(text, wideText))
            mUserDefinedAnnotation->SetMarker(wideText.Str());
    }
}

} // namespace Renderer
} // namespace NFE
