/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's compute pipeline state
 */

#include "PCH.hpp"
#include "ComputePipelineState.hpp"
#include "RendererD3D12.hpp"
#include "Shader.hpp"

#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

ComputePipelineState::ComputePipelineState()
{
}

bool ComputePipelineState::Init(const ComputePipelineStateDesc& desc)
{
    ResourceBindingLayout* resBindingLayout = dynamic_cast<ResourceBindingLayout*>(desc.resBindingLayout);
    if (!resBindingLayout)
    {
        LOG_ERROR("Invalid resource binding layout");
        return false;
    }

    Shader* computeShader = dynamic_cast<Shader*>(desc.computeShader);
    if (!computeShader)
    {
        LOG_ERROR("Invalid compute shader");
        return false;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC psd;
    ZeroMemory(&psd, sizeof(psd));
    psd.CS = computeShader->GetD3D12Bytecode();
    psd.pRootSignature = resBindingLayout->GetD3DRootSignature();
    // TODO pipeline caching

    HRESULT hr;
    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateComputePipelineState(&psd, IID_PPV_ARGS(&mPipelineState)));
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create compute pipeline state object");
        return false;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
