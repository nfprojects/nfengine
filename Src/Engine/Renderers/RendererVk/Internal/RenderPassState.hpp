#pragma once

#include "Defines.hpp"
#include "CommandRecording.hpp"

#include <Engine/Common/Containers/StaticArray.hpp>


namespace NFE {
namespace Renderer {

class RenderPassState
{
    struct Snapshot
    {
        VkRect2D mRenderPassRect;
        VkRenderPass mRenderPass;
        VkFramebuffer mFramebuffer;
        uint32 mStartBatchID;
        uint32 mEndBatchID;

        Snapshot()
            : mRenderPassRect()
            , mRenderPass(VK_NULL_HANDLE)
            , mFramebuffer(VK_NULL_HANDLE)
            , mStartBatchID(NFE_VK_INVALID_BATCH)
            , mEndBatchID(NFE_VK_INVALID_BATCH)
        {
        }

        Snapshot(VkRect2D renderPassRect, VkRenderPass renderPass, VkFramebuffer framebuffer,
                 uint32 startBatchID, uint32 endBatchID)
            : mRenderPassRect(renderPassRect)
            , mRenderPass(renderPass)
            , mFramebuffer(framebuffer)
            , mStartBatchID(startBatchID)
            , mEndBatchID(endBatchID)
        {
        }
    };

    Common::StaticArray<Snapshot, NFE_VK_MAX_RENDER_PASS_STATES> mStates;

public:
    RenderPassState();

    void Register(VkRect2D rect, VkRenderPass renderPass, VkFramebuffer framebuffer, uint32 batch);
    void End(uint32 batch);
    void Submit(CommandRecording& recording);
    void Print() const
    {
        NFE_LOG_DEBUG("Registered Render Passes:");
        for (uint32 i = 0; i < mStates.Size(); ++i)
        {
            NFE_LOG_DEBUG(" -> %dx%d; batches %d -> %d",
                          mStates[i].mRenderPassRect.extent.width,
                          mStates[i].mRenderPassRect.extent.height,
                          mStates[i].mStartBatchID,
                          mStates[i].mEndBatchID);
        }
    }
};

} // namespace Renderer
} // namespace NFE
