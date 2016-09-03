/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Direct3D 12 render's pipeline states.
 */

#pragma once

#include "../RendererInterface/PipelineState.hpp"
#include "Common.hpp"
#include "ResourceBinding.hpp"

namespace NFE {
namespace Renderer {

class PipelineState : public IPipelineState
{
    D3DPtr<ID3D12PipelineState> mPipelineState;
    ResourceBindingLayout* mBindingLayout;
    D3D12_PRIMITIVE_TOPOLOGY mPrimitiveTopology;

public:
    PipelineState();

    bool Init(const PipelineStateDesc& desc);

    NFE_INLINE ID3D12PipelineState* GetPSO() const
    {
        return mPipelineState.get();
    }

    NFE_INLINE ResourceBindingLayout* GetResBindingLayout() const
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
