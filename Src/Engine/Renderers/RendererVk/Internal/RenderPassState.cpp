#include "../PCH.hpp"
#include "RenderPassState.hpp"


namespace NFE {
namespace Renderer {


RenderPassState::RenderPassState()
    : mStates()
{
}

void RenderPassState::Register(VkRect2D rect, VkRenderPass renderPass, VkFramebuffer framebuffer, uint32 batch)
{
    if ((mStates.Size() > 0) && (mStates.Back().mEndBatchID == NFE_VK_INVALID_BATCH))
        mStates.Back().mEndBatchID = batch;

    NFE_ASSERT(mStates.Size() <= NFE_VK_MAX_RENDER_PASS_STATES, "Render Pass states collection full");

    // TODO replace with EmplaceBack when it's implemented
    mStates.PushBack(Snapshot(rect, renderPass, framebuffer, batch, NFE_VK_INVALID_BATCH));
}

void RenderPassState::End(uint32 batch)
{
    mStates.Back().mEndBatchID = batch;
}

void RenderPassState::Submit(CommandRecording& recording)
{
    // If all Render Pass snapshots were registered correctly, we should have
    // Cases to cover:
    //  - A Render Pass should end before any transfer-related commands are called
    //  - A Render Pass should be restarted if Render Target did not change and more rendering is
    //    done after a transfer command
    for (uint32 i = 0; i < mStates.Size(); ++i)
    {
        Snapshot& s = mStates[i];

    }
}


} // namespace Renderer
} // namespace NFE
