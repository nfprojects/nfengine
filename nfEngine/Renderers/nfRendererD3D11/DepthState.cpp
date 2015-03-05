/**
 * @file    DepthState.cpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's depth state
 */

#include "stdafx.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

bool DepthState::Init(const DepthStateDesc& desc)
{
    D3D11_DEPTH_STENCIL_DESC dsd;
    dsd.DepthEnable = desc.depthTestEnable;
    dsd.DepthWriteMask = desc.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL :
                                                 D3D11_DEPTH_WRITE_MASK_ZERO;
    dsd.DepthFunc = TranslateComparisonFunc(desc.depthCompareFunc);

    // TODO: stencil buffer support
    const D3D11_DEPTH_STENCILOP_DESC defaultStencilOp =
    {
        D3D11_STENCIL_OP_KEEP,
        D3D11_STENCIL_OP_KEEP,
        D3D11_STENCIL_OP_KEEP,
        D3D11_COMPARISON_ALWAYS
    };
    dsd.FrontFace = defaultStencilOp;
    dsd.BackFace = defaultStencilOp;
    dsd.StencilEnable = FALSE;
    dsd.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    HRESULT hr = gDevice->Get()->CreateDepthStencilState(&dsd, &mDS);
    return SUCCEEDED(hr);
}

} // namespace Renderer
} // namespace NFE
