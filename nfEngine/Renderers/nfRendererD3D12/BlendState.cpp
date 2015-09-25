/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's blend state
 */

#include "PCH.hpp"
#include "PipelineState.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"

namespace NFE {
namespace Renderer {

bool BlendState::Init(const BlendStateDesc& desc)
{
    UNUSED(desc);
    return false;
}

} // namespace Renderer
} // namespace NFE
