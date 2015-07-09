/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's rasterizer state
 */

#include "PCH.hpp"
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
        case CullMode::None:
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

    rd.FrontCounterClockwise = FALSE;
    rd.DepthBias = FALSE;
    rd.DepthBiasClamp = 0.0f;
    rd.SlopeScaledDepthBias = 0.0f;
    rd.DepthClipEnable = TRUE;
    rd.ScissorEnable = FALSE;
    rd.MultisampleEnable = FALSE;
    rd.AntialiasedLineEnable = FALSE;

    HRESULT hr = D3D_CALL_CHECK(gDevice->Get()->CreateRasterizerState(&rd, &mRS));
    return SUCCEEDED(hr);
}

} // namespace Renderer
} // namespace NFE
