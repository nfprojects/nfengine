/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's pipeline state
 */

#include "PCH.hpp"
#include "PipelineState.hpp"
#include "RendererD3D11.hpp"
#include "Translations.hpp"

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

#ifdef D3D_DEBUGGING
    /// set debug name
    std::string bufferName = "NFE::Renderer::RasterizerState \"";
    if (desc.debugName)
        bufferName += desc.debugName;
    bufferName += '"';
    rasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName,
                                    static_cast<UINT>(bufferName.length()),
                                    bufferName.c_str());
#endif // D3D_DEBUGGING

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

#ifdef D3D_DEBUGGING
    /// set debug name
    std::string bufferName = "NFE::Renderer::BlendState \"";
    if (desc.debugName)
        bufferName += desc.debugName;
    bufferName += '"';
    blendState->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(bufferName.length()),
                               bufferName.c_str());
#endif // D3D_DEBUGGING

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

#ifdef D3D_DEBUGGING
    /// set debug name
    std::string bufferName = "NFE::Renderer::DepthState \"";
    if (desc.debugName)
        bufferName += desc.debugName;
    bufferName += '"';
    blendState->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(bufferName.length()),
                               bufferName.c_str());
#endif // D3D_DEBUGGING

    return blendState;
}

} // namespace


bool PipelineState::Init(const PipelineStateDesc& desc)
{
    mRS = CreateRasterizerState(desc.raterizerState);
    mBS = CreateBlendState(desc.blendState);
    mDS = CreateDepthState(desc.depthState);

    if (!mRS || !mBS || !mDS)
    {
        mRS.reset();
        mBS.reset();
        mDS.reset();
        return false;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
