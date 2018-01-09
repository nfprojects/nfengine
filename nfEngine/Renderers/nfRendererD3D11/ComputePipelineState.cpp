/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's compute pipeline state
 */

#include "PCH.hpp"
#include "ComputePipelineState.hpp"
#include "RendererD3D11.hpp"
#include "Shader.hpp"

#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

ComputePipelineState::ComputePipelineState()
    : mComputeShader(nullptr)
{
}

ComputePipelineState::~ComputePipelineState()
{
    Release();
}

void ComputePipelineState::Release()
{
    mResBindingLayout.Reset();
    D3D_SAFE_RELEASE(mComputeShader);
}

bool ComputePipelineState::Init(const ComputePipelineStateDesc& desc)
{
    mResBindingLayout = desc.resBindingLayout;
    if (!mResBindingLayout)
    {
        NFE_LOG_ERROR("Invalid shader resource binding layout");
        return false;
    }

    Shader* computeShader = dynamic_cast<Shader*>(desc.computeShader.Get());

    if (computeShader)
        mComputeShader = static_cast<ID3D11ComputeShader*>(computeShader->GetShaderObject());

    if (!mComputeShader)
    {
        NFE_LOG_ERROR("Compute shader must be provided when creating compute pipeline state object");
        return false;
    }

    // additional reference in case of IShader is released
    mComputeShader->AddRef();
    return true;
}

} // namespace Renderer
} // namespace NFE
