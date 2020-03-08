/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Direct3D 12 render's pipeline states.
 */

#pragma once

#include "../Interface/PipelineState.hpp"
#include "Common.hpp"
#include "ResourceBinding.hpp"
#include "Shader.hpp"


namespace NFE {
namespace Renderer {

class PipelineState : public IPipelineState
{
    D3DPtr<ID3D12PipelineState> mPipelineState;
    D3D12_PRIMITIVE_TOPOLOGY mPrimitiveTopology;

    InternalResourceBindingLayoutPtr mBindingLayout;
    InternalShaderPtr mVS;
    InternalShaderPtr mPS;
    InternalShaderPtr mGS;
    InternalShaderPtr mHS;
    InternalShaderPtr mDS;

    void Release();

public:
    PipelineState();

    bool Init(const PipelineStateDesc& desc);

    NFE_INLINE ID3D12PipelineState* GetPSO() const
    {
        return mPipelineState.Get();
    }

    NFE_INLINE const InternalResourceBindingLayoutPtr& GetResBindingLayout() const
    {
        return mBindingLayout;
    }

    NFE_INLINE D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const
    {
        return mPrimitiveTopology;
    }
};

} // namespace Renderer
} // namespace NFE
