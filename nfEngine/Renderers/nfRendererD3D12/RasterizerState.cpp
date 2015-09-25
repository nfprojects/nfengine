/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's rasterizer state
 */

#include "PCH.hpp"
#include "PipelineState.hpp"
#include "RendererD3D12.hpp"

namespace NFE {
namespace Renderer {

bool RasterizerState::Init(const RasterizerStateDesc& desc)
{
    UNUSED(desc);
    return false;
}

} // namespace Renderer
} // namespace NFE
