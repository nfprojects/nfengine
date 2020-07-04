#pragma once

#include "API/RenderTarget.hpp"
#include "API/Buffer.hpp"


namespace NFE {
namespace Renderer {


struct CommandBufferState
{
    bool mInsideRenderPass;
    VkPipelineLayout mPipelineLayout;
    VkRect2D mRenderTargetRect;
    Buffer* mBoundVolatileBuffers[NFE_VK_MAX_VOLATILE_BUFFERS];
    uint32 mBoundVolatileOffsets[NFE_VK_MAX_VOLATILE_BUFFERS];

    CommandBufferState()
        : mInsideRenderPass(false)
        , mPipelineLayout(VK_NULL_HANDLE)
        , mRenderTargetRect()
        , mBoundVolatileBuffers()
        , mBoundVolatileOffsets()
    {
    }

    void Clear()
    {
        mInsideRenderPass = false;
        mPipelineLayout = VK_NULL_HANDLE;
        VK_ZERO_MEMORY(mRenderTargetRect);
        VK_ZERO_MEMORY(mBoundVolatileBuffers);
        VK_ZERO_MEMORY(mBoundVolatileOffsets);
    }
};


} // namespace Renderer
} // namespace NFE
