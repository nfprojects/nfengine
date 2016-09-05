/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's compute pipeline state
 */

#include "PCH.hpp"
#include "ComputePipelineState.hpp"
#include "RendererD3D11.hpp"
#include "Shader.hpp"

#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

ComputePipelineState::ComputePipelineState()
    : mComputeShader(nullptr)
    , mResBindingLayout(nullptr)
{
}

bool ComputePipelineState::Init(const ComputePipelineStateDesc& desc)
{
    UNUSED(desc);
    return false;
}

} // namespace Renderer
} // namespace NFE
