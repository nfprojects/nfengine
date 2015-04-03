/**
 * @file    CommandBuffer.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's command buffer
 */

#include "stdafx.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

CommandBuffer::CommandBuffer(ID3D11DeviceContext* deviceContext)
    : mContext(deviceContext)
    , mCurrentPrimitiveType(PrimitiveType::Unknown)
    , mCurrentRenderTarget(nullptr)
{
}

CommandBuffer::~CommandBuffer()
{
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

void CommandBuffer::SetIndexBuffer(IBuffer* indexBuffer)
{
}

void CommandBuffer::SetSamplers(ISampler** samplers, int num, ShaderType target)
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
        mContext->VSSetSamplers(0, num, samplerStates);
        break;
    case ShaderType::Domain:
        mContext->DSSetSamplers(0, num, samplerStates);
        break;
    case ShaderType::Hull:
        mContext->HSSetSamplers(0, num, samplerStates);
        break;
    case ShaderType::Geometry:
        mContext->GSSetSamplers(0, num, samplerStates);
        break;
    case ShaderType::Pixel:
        mContext->PSSetSamplers(0, num, samplerStates);
        break;
    };
}

void CommandBuffer::SetTextures(ITexture** textures, int num, ShaderType target)
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
        mContext->VSSetShaderResources(0, num, srvs);
        break;
    case ShaderType::Domain:
        mContext->DSSetShaderResources(0, num, srvs);
        break;
    case ShaderType::Hull:
        mContext->HSSetShaderResources(0, num, srvs);
        break;
    case ShaderType::Geometry:
        mContext->GSSetShaderResources(0, num, srvs);
        break;
    case ShaderType::Pixel:
        mContext->PSSetShaderResources(0, num, srvs);
        break;
    };
}

void CommandBuffer::SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target)
{
}

void CommandBuffer::SetRenderTarget(IRenderTarget* renderTarget)
{
    RenderTarget* rt = dynamic_cast<RenderTarget*>(renderTarget);

    if (rt == mCurrentRenderTarget)
        return;

    if (rt == nullptr)
    {
        mContext->OMSetRenderTargets(0, NULL, NULL);
        mCurrentRenderTarget = rt;
        return;
    }

    ID3D11RenderTargetView* rtvs[MAX_RENDER_TARGETS] = { nullptr };
    int num = rt->mRTVs.size();

    for (int i = 0; i < num; ++i)
        rtvs[i] = rt->mRTVs[i].get();

    mContext->OMSetRenderTargets(num, rtvs, rt->mDSV.get());
    mCurrentRenderTarget = rt;
}

void CommandBuffer::SetShaderProgram(IShaderProgram* shaderProgram)
{
    const ShaderProgramDesc& newProg = dynamic_cast<ShaderProgram*>(shaderProgram)->GetDesc();

    if (newProg.vertexShader != mBoundShaders.vertexShader)
    {
        Shader* shader = dynamic_cast<Shader*>(newProg.vertexShader);
        mContext->VSSetShader((ID3D11VertexShader*)shader->GetShaderObject(), nullptr, 0);
        mBoundShaders.vertexShader = newProg.vertexShader;
    }

    if (newProg.geometryShader != mBoundShaders.geometryShader)
    {
        Shader* shader = dynamic_cast<Shader*>(newProg.geometryShader);
        mContext->GSSetShader((ID3D11GeometryShader*)shader->GetShaderObject(), nullptr, 0);
        mBoundShaders.geometryShader = newProg.geometryShader;
    }

    if (newProg.hullShader != mBoundShaders.hullShader)
    {
        Shader* shader = dynamic_cast<Shader*>(newProg.hullShader);
        mContext->HSSetShader((ID3D11HullShader*)shader->GetShaderObject(), nullptr, 0);
        mBoundShaders.hullShader = newProg.hullShader;
    }

    if (newProg.domainShader != mBoundShaders.domainShader)
    {
        Shader* shader = dynamic_cast<Shader*>(newProg.domainShader);
        mContext->DSSetShader((ID3D11DomainShader*)shader->GetShaderObject(), nullptr, 0);
        mBoundShaders.domainShader = newProg.domainShader;
    }

    if (newProg.pixelShader != mBoundShaders.pixelShader)
    {
        Shader* shader = dynamic_cast<Shader*>(newProg.pixelShader);
        mContext->PSSetShader((ID3D11PixelShader*)shader->GetShaderObject(), nullptr, 0);
        mBoundShaders.pixelShader = newProg.pixelShader;
    }
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

void CommandBuffer::CopyTexture(ITexture* src, ITexture* dest)
{
    Texture* srcTex = dynamic_cast<Texture*>(src);
    Texture* destTex = dynamic_cast<Texture*>(dest);

    if (!srcTex || !destTex)
        return;

    mContext->CopyResource(reinterpret_cast<ID3D11Resource*>(destTex->mTextureGeneric),
                           reinterpret_cast<ID3D11Resource*>(srcTex->mTextureGeneric));
}

bool CommandBuffer::ReadTexture(ITexture* tex, void* data)
{
    Texture* texture = dynamic_cast<Texture*>(tex);
    if (!texture)
        return false;

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

void CommandBuffer::Clear(const float* color)
{
    // TODO: what about cleaning individual RTs with different colors?

    if (mCurrentRenderTarget)
        for (int i = 0; i < mCurrentRenderTarget->mRTVs.size(); ++i)
            mContext->ClearRenderTargetView(mCurrentRenderTarget->mRTVs[i].get(), color);
}

void CommandBuffer::Draw(PrimitiveType type, int vertexNum, int instancesNum,
                         int indexOffset, int vertexOffset, int instanceOffset)
{
    if (type != mCurrentPrimitiveType)
    {
        D3D11_PRIMITIVE_TOPOLOGY topology;
        switch (type)
        {
            case PrimitiveType::Points:
                topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
                break;
            case PrimitiveType::Lines:
                topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
                break;
            case PrimitiveType::LinesStrip:
                topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
                break;
            case PrimitiveType::Triangles:
                topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                break;
            case PrimitiveType::TrianglesStrip:
                topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                break;
            default:
                topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        };
        mContext->IASetPrimitiveTopology(topology);
        mCurrentPrimitiveType = type;
    };

    /*
     TODO:
     * instancing support
     * indexing support
     */
    mContext->Draw(vertexNum, vertexOffset);
}

void CommandBuffer::Execute(ICommandBuffer* commandBuffer, bool saveState)
{
}

} // namespace Renderer
} // namespace NFE
