/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Direct3D 11 render's compute pipeline states.
 */

#pragma once

#include "../RendererCommon/ComputePipelineState.hpp"
#include "Common.hpp"
#include "ResourceBinding.hpp"
#include "Shader.hpp"


namespace NFE {
namespace Renderer {

class ComputePipelineState : public IComputePipelineState
{
    D3DPtr<ID3D12PipelineState> mPipelineState;

    // keep reference to compute shader bytecode
    InternalShaderPtr mComputeShader;

    // keeps reference to root signature
    InternalResourceBindingLayoutPtr mResBindingLayout;

    void Release();

public:
    ComputePipelineState();
    bool Init(const ComputePipelineStateDesc& desc);

    NFE_INLINE ID3D12PipelineState* GetPSO() const
    {
        return mPipelineState.Get();
    }
};

} // namespace Renderer
} // namespace NFE
