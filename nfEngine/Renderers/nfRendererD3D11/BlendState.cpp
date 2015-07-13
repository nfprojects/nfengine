/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's blend state
 */

#include "PCH.hpp"
#include "PipelineState.hpp"
#include "RendererD3D11.hpp"
#include "Translations.hpp"

namespace NFE {
namespace Renderer {

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
    if (FAILED(hr))
        return false;

#ifdef D3D_DEBUGGING
    /// set debug name
    std::string bufferName = "NFE::Renderer::BlendState \"";
    if (desc.debugName)
        bufferName += desc.debugName;
    bufferName += '"';
    mBS->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(bufferName.length()),
                        bufferName.c_str());
#endif // D3D_DEBUGGING

    return true;
}

} // namespace Renderer
} // namespace NFE
