#pragma once

#include "API/RenderTarget.hpp"
#include "API/Buffer.hpp"


namespace NFE {
namespace Renderer {


struct CommandBatchState
{
    VkPipelineLayout mPipelineLayout;
    VkRect2D mRenderTargetRect;

    CommandBatchState()
        : mPipelineLayout(VK_NULL_HANDLE)
        , mRenderTargetRect()
    {
    }

    void Clear()
    {
        mPipelineLayout = VK_NULL_HANDLE;
        VK_ZERO_MEMORY(mRenderTargetRect);
    }
};


} // namespace Renderer
} // namespace NFE
