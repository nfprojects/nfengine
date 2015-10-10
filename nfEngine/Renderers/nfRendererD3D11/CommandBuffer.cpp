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

void CommandBuffer::SetSamplers(ISampler** samplers, int num, ShaderType target, int slotOffset)
{
    ID3D11SamplerState* samplerStates[16];
    for (int i = 0; i < num; ++i)
    {
        Sampler* sampler = dynamic_cast<Sampler*>(samplers[i]);
        samplerStates[i] = sampler->mSamplerState.get();
    }

    switch (target)
    {
    case ShaderType::Vertex:
        mContext->VSSetSamplers(slotOffset, num, samplerStates);
        break;
    case ShaderType::Domain:
        mContext->DSSetSamplers(slotOffset, num, samplerStates);
        break;
    case ShaderType::Hull:
        mContext->HSSetSamplers(slotOffset, num, samplerStates);
        break;
    case ShaderType::Geometry:
        mContext->GSSetSamplers(slotOffset, num, samplerStates);
        break;
    case ShaderType::Pixel:
        mContext->PSSetSamplers(slotOffset, num, samplerStates);
        break;
    };
}

void CommandBuffer::SetTextures(ITexture** textures, int num, ShaderType target, int slotOffset)
{
    ID3D11ShaderResourceView* srvs[16];
    for (int i = 0; i < num; ++i)
    {
        Texture* texture = dynamic_cast<Texture*>(textures[i]);
        srvs[i] = texture->mSRV.get();
    }

    switch (target)
    {
    case ShaderType::Vertex:
        mContext->VSSetShaderResources(slotOffset, num, srvs);
        break;
    case ShaderType::Domain:
        mContext->DSSetShaderResources(slotOffset, num, srvs);
        break;
    case ShaderType::Hull:
        mContext->HSSetShaderResources(slotOffset, num, srvs);
        break;
    case ShaderType::Geometry:
        mContext->GSSetShaderResources(slotOffset, num, srvs);
        break;
    case ShaderType::Pixel:
        mContext->PSSetShaderResources(slotOffset, num, srvs);
        break;
    };
}

void CommandBuffer::SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target,
                                       int slotOffset)
{
    ID3D11Buffer* buffers[16];
    for (int i = 0; i < num; ++i)
    {
        Buffer* cb = dynamic_cast<Buffer*>(constantBuffers[i]);
        buffers[i] = cb ? cb->mBuffer.get() : NULL;
    }

    switch (target)
    {
        case ShaderType::Vertex:
            mContext->VSSetConstantBuffers(slotOffset, num, buffers);
            break;
        case ShaderType::Domain:
            mContext->DSSetConstantBuffers(slotOffset, num, buffers);
            break;
        case ShaderType::Hull:
            mContext->HSSetConstantBuffers(slotOffset, num, buffers);
            break;
        case ShaderType::Geometry:
            mContext->GSSetConstantBuffers(slotOffset, num, buffers);
            break;
        case ShaderType::Pixel:
            mContext->PSSetConstantBuffers(slotOffset, num, buffers);
            break;
    };
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
    const ShaderProgramDesc& newProg = dynamic_cast<ShaderProgram*>(shaderProgram)->GetDesc();

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

ICommandList* CommandBuffer::Finish()
{
    HRESULT hr;
    ID3D11CommandList* d3dList;
    hr = D3D_CALL_CHECK(mContext->FinishCommandList(FALSE, &d3dList));
    if (FAILED(hr))
        return nullptr;

    // TODO: use memory pool
    CommandList* list = new (std::nothrow) CommandList;
    if (!list)
    {
        D3D_SAFE_RELEASE(d3dList);
        LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    list->mD3DList = d3dList;
    return list;
}

void CommandBuffer::Execute(ICommandList* commandList)
{
    CommandList* list = dynamic_cast<CommandList*>(commandList);
    if (!list)
        return;

    mContext->ExecuteCommandList(list->mD3DList.get(), FALSE);
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
