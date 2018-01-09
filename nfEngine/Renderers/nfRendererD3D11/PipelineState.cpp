/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's pipeline state
 */

#include "PCH.hpp"
#include "PipelineState.hpp"
#include "RendererD3D11.hpp"
#include "Translations.hpp"
#include "VertexLayout.hpp"
#include "Shader.hpp"

#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

namespace {

ID3D11RasterizerState* CreateRasterizerState(const RasterizerStateDesc& desc)
{
    ID3D11RasterizerState* rasterizerState;
    D3D11_RASTERIZER_DESC rd;

    switch (desc.cullMode)
    {
    case CullMode::CW:
        rd.CullMode = D3D11_CULL_BACK;
        break;
    case CullMode::CCW:
        rd.CullMode = D3D11_CULL_FRONT;
        break;
    case CullMode::Disabled:
        rd.CullMode = D3D11_CULL_NONE;
        break;
    default:
        return false;
    };

    switch (desc.fillMode)
    {
    case FillMode::Solid:
        rd.FillMode = D3D11_FILL_SOLID;
        break;
    case FillMode::Wireframe:
        rd.FillMode = D3D11_FILL_WIREFRAME;
        break;
    default:
        return false;
    };

    rd.FrontCounterClockwise = TRUE;
    rd.DepthBias = FALSE;
    rd.DepthBiasClamp = 0.0f;
    rd.SlopeScaledDepthBias = 0.0f;
    rd.DepthClipEnable = TRUE;
    rd.ScissorEnable = desc.scissorTest ? TRUE : FALSE;
    rd.MultisampleEnable = FALSE;
    rd.AntialiasedLineEnable = FALSE;

    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateRasterizerState(&rd, &rasterizerState));
    if (FAILED(hr))
        return false;

    return rasterizerState;
}

ID3D11BlendState* CreateBlendState(const BlendStateDesc& desc)
{
    ID3D11BlendState* blendState;

    D3D11_BLEND_DESC bd;
    bd.AlphaToCoverageEnable = desc.alphaToCoverage;
    bd.IndependentBlendEnable = desc.independent;

    int iterations = desc.independent ? MAX_RENDER_TARGETS : 1;
    for (int i = 0; i < iterations; ++i)
    {
        const RenderTargetBlendStateDesc& rtDesc = desc.rtDescs[i];

        bd.RenderTarget[i].BlendEnable = rtDesc.enable;
        bd.RenderTarget[i].SrcBlend = TranslateBlendFunc(rtDesc.srcColorFunc);
        bd.RenderTarget[i].DestBlend = TranslateBlendFunc(rtDesc.destColorFunc);
        bd.RenderTarget[i].SrcBlendAlpha = TranslateBlendFunc(rtDesc.srcAlphaFunc);
        bd.RenderTarget[i].DestBlendAlpha = TranslateBlendFunc(rtDesc.destAlphaFunc);
        bd.RenderTarget[i].BlendOp = TranslateBlendOp(rtDesc.colorOperator);
        bd.RenderTarget[i].BlendOpAlpha = TranslateBlendOp(rtDesc.alphaOperator);
        bd.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }

    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateBlendState(&bd, &blendState));
    if (FAILED(hr))
        return false;

    return blendState;
}

ID3D11DepthStencilState* CreateDepthState(const DepthStateDesc& desc)
{
    ID3D11DepthStencilState* blendState;
    D3D11_DEPTH_STENCIL_DESC dsd;

    dsd.DepthEnable = desc.depthTestEnable;
    dsd.DepthWriteMask = desc.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL :
        D3D11_DEPTH_WRITE_MASK_ZERO;
    dsd.DepthFunc = TranslateComparisonFunc(desc.depthCompareFunc);

    // TODO: stencil buffer support
    const D3D11_DEPTH_STENCILOP_DESC stencilOpDesc =
    {
        TranslateStencilOp(desc.stencilOpFail),
        TranslateStencilOp(desc.stencilOpDepthFail),
        TranslateStencilOp(desc.stencilOpPass),
        TranslateComparisonFunc(desc.stencilFunc),
    };
    dsd.FrontFace = dsd.BackFace = stencilOpDesc;
    dsd.StencilEnable = desc.stencilEnable;
    dsd.StencilReadMask = desc.stencilMask;
    dsd.StencilWriteMask = desc.stencilMask;

    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateDepthStencilState(&dsd, &blendState));
    if (FAILED(hr))
        return false;

    return blendState;
}

} // namespace


PipelineState::PipelineState()
    : mVertexShader(nullptr)
    , mGeometryShader(nullptr)
    , mHullShader(nullptr)
    , mDomainShader(nullptr)
    , mPixelShader(nullptr)
{
}

PipelineState::~PipelineState()
{
    Release();
}

void PipelineState::Release()
{
    mResBindingLayout.Reset();
    mVertexLayout.Reset();
    mRS.Reset();
    mBS.Reset();
    mDS.Reset();

    D3D_SAFE_RELEASE(mVertexShader);
    D3D_SAFE_RELEASE(mGeometryShader);
    D3D_SAFE_RELEASE(mHullShader);
    D3D_SAFE_RELEASE(mDomainShader);
    D3D_SAFE_RELEASE(mPixelShader);
}

bool PipelineState::Init(const PipelineStateDesc& desc)
{
    mPrimitiveType = desc.primitiveType;
    mNumControlPoints = desc.numControlPoints;

    mResBindingLayout = desc.resBindingLayout;
    if (!mResBindingLayout)
    {
        Release();
        NFE_LOG_ERROR("Invalid shader resource binding layout");
        return false;
    }

    mVertexLayout = Common::StaticCast<VertexLayout>(desc.vertexLayout);
    if (!mVertexLayout)
    {
        Release();
        NFE_LOG_ERROR("Invalid vertexLayout parameter");
        return false;
    }

    mRS = CreateRasterizerState(desc.raterizerState);
    mBS = CreateBlendState(desc.blendState);
    mDS = CreateDepthState(desc.depthState);

    if (!mRS || !mBS || !mDS)
    {
        Release();
        return false;
    }

    // set debug name
    if (gDevice->IsDebugLayerEnabled() && desc.debugName)
    {
        const size_t MAX_DEBUG_NAME_LEN = 128;
        char buffer[MAX_DEBUG_NAME_LEN];
        int len;

        len = snprintf(buffer, MAX_DEBUG_NAME_LEN, "RasterizerState '%s'", desc.debugName);
        D3D_CALL_CHECK(mRS->SetPrivateData(WKPDID_D3DDebugObjectName, len, buffer));

        len = snprintf(buffer, MAX_DEBUG_NAME_LEN, "BlendState '%s'", desc.debugName);
        D3D_CALL_CHECK(mBS->SetPrivateData(WKPDID_D3DDebugObjectName, len, buffer));

        len = snprintf(buffer, MAX_DEBUG_NAME_LEN, "DepthState '%s'", desc.debugName);
        D3D_CALL_CHECK(mDS->SetPrivateData(WKPDID_D3DDebugObjectName, len, buffer));
    }

    Shader* vertexShader = dynamic_cast<Shader*>(desc.vertexShader.Get());
    Shader* geometryShader = dynamic_cast<Shader*>(desc.geometryShader.Get());
    Shader* hullShader = dynamic_cast<Shader*>(desc.hullShader.Get());
    Shader* domainShader = dynamic_cast<Shader*>(desc.domainShader.Get());
    Shader* pixelShader = dynamic_cast<Shader*>(desc.pixelShader.Get());

    if (vertexShader)
    {
        mVertexShader = static_cast<ID3D11VertexShader*>(vertexShader->GetShaderObject());

        // additional reference in case of IShader is released
        if (mVertexShader)
            mVertexShader->AddRef();
    }

    // Pipeline State must have at least vertex shader
    if (!mVertexShader)
    {
        Release();
        NFE_LOG_ERROR("Vertex shader must be provided when creating pipeline state object");
        return false;
    }

    if (geometryShader)
    {
        mGeometryShader = static_cast<ID3D11GeometryShader*>(geometryShader->GetShaderObject());

        // additional reference in case of IShader is released
        if (mGeometryShader)
            mGeometryShader->AddRef();
    }

    if (hullShader)
    {
        mHullShader = static_cast<ID3D11HullShader*>(hullShader->GetShaderObject());

        // additional reference in case of IShader is released
        if (mHullShader)
            mHullShader->AddRef();
    }

    if (domainShader)
    {
        mDomainShader = static_cast<ID3D11DomainShader*>(domainShader->GetShaderObject());

        // additional reference in case of IShader is released
        if (mDomainShader)
            mDomainShader->AddRef();
    }

    if (pixelShader)
    {
        mPixelShader = static_cast<ID3D11PixelShader*>(pixelShader->GetShaderObject());

        // additional reference in case of IShader is released
        if (mPixelShader)
            mPixelShader->AddRef();
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
