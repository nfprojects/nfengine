/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's depth state
 */

#include "PCH.hpp"
#include "PipelineState.hpp"
#include "RendererD3D11.hpp"
#include "Translations.hpp"

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

    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateDepthStencilState(&dsd, &mDS));
    if (FAILED(hr))
        return false;

#ifdef D3D_DEBUGGING
    /// set debug name
    std::string bufferName = "NFE::Renderer::DepthState \"";
    if (desc.debugName)
        bufferName += desc.debugName;
    bufferName += '"';
    mDS->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(bufferName.length()),
                        bufferName.c_str());
#endif // D3D_DEBUGGING

    return true;
}

} // namespace Renderer
} // namespace NFE
