/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's blend state
 */

#include "PCH.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

namespace {

D3D11_BLEND TranslateBlendFunc(BlendFunc func)
{
    switch (func)
    {
        case BlendFunc::Zero:
            return D3D11_BLEND_ZERO;
        case BlendFunc::One:
            return D3D11_BLEND_ONE;

        case BlendFunc::SrcColor:
            return D3D11_BLEND_SRC_COLOR;
        case BlendFunc::DestColor:
            return D3D11_BLEND_DEST_COLOR;
        case BlendFunc::SrcAlpha:
            return D3D11_BLEND_SRC_ALPHA;
        case BlendFunc::DestAlpha:
            return D3D11_BLEND_DEST_ALPHA;

        case BlendFunc::OneMinusSrcColor:
            return D3D11_BLEND_INV_SRC_COLOR;
        case BlendFunc::OneMinusDestColor:
            return D3D11_BLEND_INV_DEST_COLOR;
        case BlendFunc::OneMinusSrcAlpha:
            return D3D11_BLEND_INV_SRC_ALPHA;
        case BlendFunc::OneMinusDestAlpha:
            return D3D11_BLEND_INV_DEST_ALPHA;
    };

    return D3D11_BLEND_ZERO;
}

D3D11_BLEND_OP TranslateBlendOp(BlendOp op)
{
    switch (op)
    {
        case BlendOp::Add:
            return D3D11_BLEND_OP_ADD;
        case BlendOp::Subtract:
            return D3D11_BLEND_OP_SUBTRACT;
        case BlendOp::RevSubtract:
            return D3D11_BLEND_OP_REV_SUBTRACT;
        case BlendOp::Min:
            return D3D11_BLEND_OP_MIN;
        case BlendOp::Max:
            return D3D11_BLEND_OP_MAX;
    };

    return D3D11_BLEND_OP_ADD;
}

} // namespace

bool BlendState::Init(const BlendStateDesc& desc)
{
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

    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateBlendState(&bd, &mBS));
    return SUCCEEDED(hr);
}

} // namespace Renderer
} // namespace NFE
