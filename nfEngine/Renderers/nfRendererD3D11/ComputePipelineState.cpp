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
    mResBindingLayout = dynamic_cast<ResourceBindingLayout*>(desc.resBindingLayout);
    if (!mResBindingLayout)
    {
        LOG_ERROR("Invalid shader resource binding layout");
        return false;
    }

    Shader* computeShader = dynamic_cast<Shader*>(desc.computeShader);

    if (computeShader)
        mComputeShader = static_cast<ID3D11ComputeShader*>(computeShader->GetShaderObject());

    if (!mComputeShader)
    {
        LOG_ERROR("Compute shader must be provided when creating compute pipeline state object");
        return false;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
