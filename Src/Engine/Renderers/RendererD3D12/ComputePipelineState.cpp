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

void ComputePipelineState::Release()
{
    mPipelineState.Reset();
    mComputeShader.Reset();
}

bool ComputePipelineState::Init(const ComputePipelineStateDesc& desc)
{
    mComputeShader = Common::StaticCast<Shader>(desc.computeShader);
    if (!mComputeShader)
    {
        NFE_LOG_ERROR("Invalid compute shader");
        return false;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC psd;
    ZeroMemory(&psd, sizeof(psd));
    psd.CS = mComputeShader->GetD3D12Bytecode();
    psd.pRootSignature = gDevice->GetComputeRootSignature();
    // TODO pipeline caching

    NFE_ASSERT(psd.pRootSignature, "Missing root signature");

    HRESULT hr;
    hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateComputePipelineState(&psd, IID_PPV_ARGS(mPipelineState.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create compute pipeline state object");
        return false;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
