/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's compute pipeline state
 */

#include "PCH.hpp"
#include "ComputePipelineState.hpp"
#include "RendererD3D12.hpp"

namespace NFE {
namespace Renderer {

ComputePipelineState::ComputePipelineState()
{
}

bool ComputePipelineState::Init(const ComputePipelineStateDesc& desc)
{
    // TODO
    UNUSED(desc);
    return false;
}

} // namespace Renderer
} // namespace NFE
