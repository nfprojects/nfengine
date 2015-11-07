/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's rasterizer state
 */

#include "PCH.hpp"
#include "PipelineState.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

bool RasterizerState::Init(const RasterizerStateDesc& desc)
{
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

    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateRasterizerState(&rd, &mRS));
    if (FAILED(hr))
        return false;

#ifdef D3D_DEBUGGING
    /// set debug name
    std::string bufferName = "NFE::Renderer::RasterizerState \"";
    if (desc.debugName)
        bufferName += desc.debugName;
    bufferName += '"';
    mRS->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(bufferName.length()),
                        bufferName.c_str());
#endif // D3D_DEBUGGING

    return true;
}

} // namespace Renderer
} // namespace NFE
