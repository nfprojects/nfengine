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
#include "../../nfCommon/Logger.hpp"
#include "../../nfCommon/Win/Common.hpp"  // required for ID3DUserDefinedAnnotation

namespace NFE {
namespace Renderer {

CommandBuffer::CommandBuffer(ID3D11DeviceContext* deviceContext)
    : mContext(deviceContext)
    , mCurrentPrimitiveType(PrimitiveType::Unknown)
    , mCurrentRenderTarget(nullptr)
{
    HRESULT hr;
    hr = deviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation),
                                       reinterpret_cast<void**>(&mUserDefinedAnnotation));
    if (FAILED(hr))
        mUserDefinedAnnotation.reset();
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::Reset()
{
    mCurrentPrimitiveType = PrimitiveType::Unknown;
    mCurrentRenderTarget = nullptr;
    mBoundShaders = ShaderProgramDesc();
    mProgram = nullptr;

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

void CommandBuffer::SetVertexLayout(IVertexLayout* vertexLayout)
{
    VertexLayout* vl = dynamic_cast<VertexLayout*>(vertexLayout);
    mContext->IASetInputLayout(vl->mIL.get());
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

void CommandBuffer::SetSampler(ISampler* sampler, int slot)
{
    assert(slot >= 0 && slot < static_cast<int>(mProgram->mResBinding.samplers.size()));
    const ShaderProgramResBinding& binding = mProgram->mResBinding.samplers[slot];
    ID3D11SamplerState* d3dSampler = dynamic_cast<Sampler*>(sampler)->mSamplerState.get();

    if (binding.vertexShaderSlot >= 0)
        mContext->VSSetSamplers(binding.vertexShaderSlot, 1, &d3dSampler);
    if (binding.domainShaderSlot >= 0)
        mContext->DSSetSamplers(binding.domainShaderSlot, 1, &d3dSampler);
    if (binding.hullShaderSlot >= 0)
        mContext->HSSetSamplers(binding.hullShaderSlot, 1, &d3dSampler);
    if (binding.geometryShaderSlot >= 0)
        mContext->GSSetSamplers(binding.geometryShaderSlot, 1, &d3dSampler);
    if (binding.pixelShaderSlot >= 0)
        mContext->PSSetSamplers(binding.pixelShaderSlot, 1, &d3dSampler);
}

void CommandBuffer::SetTexture(ITexture* texture, int slot)
{
    assert(slot >= 0 && slot < static_cast<int>(mProgram->mResBinding.textures.size()));
    const ShaderProgramResBinding& binding = mProgram->mResBinding.textures[slot];
    ID3D11ShaderResourceView* d3dTexture = dynamic_cast<Texture*>(texture)->mSRV.get();

    if (binding.vertexShaderSlot >= 0)
        mContext->VSSetShaderResources(binding.vertexShaderSlot, 1, &d3dTexture);
    if (binding.domainShaderSlot >= 0)
        mContext->DSSetShaderResources(binding.domainShaderSlot, 1, &d3dTexture);
    if (binding.hullShaderSlot >= 0)
        mContext->HSSetShaderResources(binding.hullShaderSlot, 1, &d3dTexture);
    if (binding.geometryShaderSlot >= 0)
        mContext->GSSetShaderResources(binding.geometryShaderSlot, 1, &d3dTexture);
    if (binding.pixelShaderSlot >= 0)
        mContext->PSSetShaderResources(binding.pixelShaderSlot, 1, &d3dTexture);
}

void CommandBuffer::SetCBuffer(IBuffer* cbuffer, int slot)
{
    assert(slot >= 0 && slot < static_cast<int>(mProgram->mResBinding.cbuffers.size()));
    const ShaderProgramResBinding& binding = mProgram->mResBinding.cbuffers[slot];
    ID3D11Buffer* d3dBuffer = dynamic_cast<Buffer*>(cbuffer)->mBuffer.get();

    if (binding.vertexShaderSlot >= 0)
        mContext->VSSetConstantBuffers(binding.vertexShaderSlot, 1, &d3dBuffer);
    if (binding.domainShaderSlot >= 0)
        mContext->DSSetConstantBuffers(binding.domainShaderSlot, 1, &d3dBuffer);
    if (binding.hullShaderSlot >= 0)
        mContext->HSSetConstantBuffers(binding.hullShaderSlot, 1, &d3dBuffer);
    if (binding.geometryShaderSlot >= 0)
        mContext->GSSetConstantBuffers(binding.geometryShaderSlot, 1, &d3dBuffer);
    if (binding.pixelShaderSlot >= 0)
        mContext->PSSetConstantBuffers(binding.pixelShaderSlot, 1, &d3dBuffer);
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

void CommandBuffer::SetShaderProgram(IShaderProgram* shaderProgram)
{
    mProgram = dynamic_cast<ShaderProgram*>(shaderProgram);
    const ShaderProgramDesc& newProg = mProgram->GetDesc();

    if (newProg.vertexShader != mBoundShaders.vertexShader)
    {
        Shader* shader = dynamic_cast<Shader*>(newProg.vertexShader);
        mContext->VSSetShader(shader->mVS, nullptr, 0);
        mBoundShaders.vertexShader = newProg.vertexShader;
    }

    if (newProg.geometryShader != mBoundShaders.geometryShader)
    {
        Shader* shader = dynamic_cast<Shader*>(newProg.geometryShader);
        mContext->GSSetShader(shader->mGS, nullptr, 0);
        mBoundShaders.geometryShader = newProg.geometryShader;
    }

    if (newProg.hullShader != mBoundShaders.hullShader)
    {
        Shader* shader = dynamic_cast<Shader*>(newProg.hullShader);
        mContext->HSSetShader(shader->mHS, nullptr, 0);
        mBoundShaders.hullShader = newProg.hullShader;
    }

    if (newProg.domainShader != mBoundShaders.domainShader)
    {
        Shader* shader = dynamic_cast<Shader*>(newProg.domainShader);
        mContext->DSSetShader(shader->mDS, nullptr, 0);
        mBoundShaders.domainShader = newProg.domainShader;
    }

    if (newProg.pixelShader != mBoundShaders.pixelShader)
    {
        Shader* shader = dynamic_cast<Shader*>(newProg.pixelShader);
        mContext->PSSetShader(shader->mPS, nullptr, 0);
        mBoundShaders.pixelShader = newProg.pixelShader;
    }
}

void CommandBuffer::SetShader(IShader* shader)
{
    Shader* newShader = dynamic_cast<Shader*>(shader);

    switch (newShader->mType)
    {
        case ShaderType::Vertex:
            if (shader != mBoundShaders.vertexShader)
            {
                mContext->VSSetShader(newShader->mVS, nullptr, 0);
                mBoundShaders.vertexShader = shader;
            }
            break;

        case ShaderType::Geometry:
            if (shader != mBoundShaders.geometryShader)
            {
                mContext->GSSetShader(newShader->mGS, nullptr, 0);
                mBoundShaders.geometryShader = shader;
            }
            break;

        case ShaderType::Hull:
            if (shader != mBoundShaders.hullShader)
            {
                mContext->HSSetShader(newShader->mHS, nullptr, 0);
                mBoundShaders.hullShader = shader;
            }
            break;

        case ShaderType::Domain:
            if (shader != mBoundShaders.domainShader)
            {
                mContext->DSSetShader(newShader->mDS, nullptr, 0);
                mBoundShaders.domainShader = shader;
            }
            break;

        case ShaderType::Pixel:
            if (shader != mBoundShaders.pixelShader)
            {
                mContext->PSSetShader(newShader->mPS, nullptr, 0);
                mBoundShaders.pixelShader = shader;
            }
            break;
    };
}

void CommandBuffer::SetBlendState(IBlendState* state)
{
    BlendState* blendState = dynamic_cast<BlendState*>(state);
    mContext->OMSetBlendState(blendState->mBS.get(), NULL, 0xFFFFFFFF);
}

void CommandBuffer::SetRasterizerState(IRasterizerState* state)
{
    RasterizerState* rasterizerState = dynamic_cast<RasterizerState*>(state);
    mContext->RSSetState(rasterizerState->mRS.get());
}

void CommandBuffer::SetDepthState(IDepthState* state)
{
    DepthState* depthState = dynamic_cast<DepthState*>(state);
    mContext->OMSetDepthStencilState(depthState->mDS.get(), 0);
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

bool CommandBuffer::ReadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data)
{
    Buffer* buf = dynamic_cast<Buffer*>(buffer);
    if (!buf)
        return false;

    D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
    ID3D11Resource* res = reinterpret_cast<ID3D11Resource*>(buf->mBuffer.get());
    HRESULT hr = D3D_CALL_CHECK(mContext->Map(res, 0, D3D11_MAP_READ, 0, &mapped));
    if (FAILED(hr))
        return false;

    memcpy(data, (char*)mapped.pData + offset, size);
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

bool CommandBuffer::ReadTexture(ITexture* tex, void* data)
{
    Texture* texture = dynamic_cast<Texture*>(tex);
    if (!texture)
    {
        LOG_ERROR("Invalid 'tex' pointer");
        return false;
    }

    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
    UINT subresource = 0; // TODO: mipmap and layer selection
    ID3D11Resource* res = reinterpret_cast<ID3D11Resource*>(texture->mTextureGeneric);
    hr = D3D_CALL_CHECK(mContext->Map(res, subresource, D3D11_MAP_READ, 0, &mapped));
    if (FAILED(hr))
        return false;

    size_t dataSize = static_cast<size_t>(texture->mWidth) *
                      static_cast<size_t>(texture->mHeight) *
                      static_cast<size_t>(texture->mTexelSize);
    memcpy(data, mapped.pData, dataSize);

    mContext->Unmap(res, subresource);
    return true;
}

void CommandBuffer::Clear(int flags, const float* color, float depthValue)
{
    // TODO: what about cleaning individual RTs with different colors?

    if (mCurrentRenderTarget)
    {
        if (flags & NFE_CLEAR_FLAG_TARGET)
            for (size_t i = 0; i < mCurrentRenderTarget->mRTVs.size(); ++i)
                mContext->ClearRenderTargetView(mCurrentRenderTarget->mRTVs[i].get(), color);

        if (flags & NFE_CLEAR_FLAG_DEPTH)
        {
            ID3D11DepthStencilView* dsv;
            if (mCurrentRenderTarget->mDepthBuffer)
            {
                dsv = mCurrentRenderTarget->mDepthBuffer->mDSV.get();
                if (dsv)
                    mContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, depthValue, 0);
            }
        }

        // TODO: stencil buffer support
    }
}

void CommandBuffer::Draw(PrimitiveType type, int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    if (type != mCurrentPrimitiveType)
    {
        mCurrentPrimitiveType = type;
        D3D11_PRIMITIVE_TOPOLOGY topology = TranslatePrimitiveType(type);
        mContext->IASetPrimitiveTopology(topology);
    };

    if (instancesNum >= 0)
        mContext->DrawInstanced(vertexNum, instancesNum, vertexOffset, instanceOffset);
    else
        mContext->Draw(vertexNum, vertexOffset);
}

void CommandBuffer::DrawIndexed(PrimitiveType type, int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    if (type != mCurrentPrimitiveType)
    {
        mCurrentPrimitiveType = type;
        D3D11_PRIMITIVE_TOPOLOGY topology = TranslatePrimitiveType(type);
        mContext->IASetPrimitiveTopology(topology);
    };

    if (instancesNum >= 0)
        mContext->DrawIndexedInstanced(indexNum, instancesNum, indexOffset, vertexOffset,
                                       instanceOffset);
    else
        mContext->DrawIndexed(indexNum, indexOffset, vertexOffset);
}

void CommandBuffer::Execute(ICommandBuffer* commandBuffer, bool saveState)
{
    UNUSED(commandBuffer);
    UNUSED(saveState);
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
