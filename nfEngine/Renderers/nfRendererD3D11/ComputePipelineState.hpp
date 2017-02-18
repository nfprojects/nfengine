/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of Direct3D 11 render's compute pipeline states.
 */

#pragma once

#include "../RendererInterface/ComputePipelineState.hpp"
#include "Common.hpp"
#include "ResourceBinding.hpp"

namespace NFE {
namespace Renderer {

class ComputePipelineState : public IComputePipelineState
{
    ID3D11ComputeShader* mComputeShader;
    ResourceBindingLayoutPtr mResBindingLayout;

    void Release();

public:
    ComputePipelineState();
    ~ComputePipelineState();
    bool Init(const ComputePipelineStateDesc& desc);

    NFE_INLINE ID3D11ComputeShader* GetShader() const
    {
        return mComputeShader;
    }

    NFE_INLINE const ResourceBindingLayout* GetResBindingLayout() const
    {
        return static_cast<const ResourceBindingLayout*>(mResBindingLayout.get());
    }
};

} // namespace Renderer
} // namespace NFE
