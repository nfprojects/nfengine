#include "PCH.hpp"
#include "Renderer.hpp"
#include "RenderGraph.hpp"
#include "RenderGraphContext.hpp"
#include "RenderGraphNode.hpp"
#include "RenderGraphResourceAllocator.hpp"
#include "Viewport/Viewport.hpp"

// TODO command lists
// TODO parallel command list recording and execution
// TODO resource allocation

namespace NFE {
namespace Renderer {

using namespace Common;

RenderNodeGroup::RenderNodeGroup()
{
    Renderer& renderer = Renderer::GetInstance();
    mCommandRecorder = renderer.GetDevice()->CreateCommandRecorder();
}

RenderNodeGroup::~RenderNodeGroup() = default;

void RenderNodeGroup::AddNode(RenderGraphNodePtr&& node)
{
    mNodes.PushBack(std::move(node));
}

void RenderNodeGroup::Execute(RenderGraphContext::Phase phase, const FrameInfo& frameInfo, RenderGraphResourceAllocator& resourceAllocator)
{
    Renderer& renderer = Renderer::GetInstance();

    const bool isRenderPhase = phase == RenderGraphContext::Phase::Render;

    const RenderGraphContext context =
    {
        phase,
        resourceAllocator,
        frameInfo.viewport,
        isRenderPhase ? mCommandRecorder.Get() : nullptr,
    };

    if (isRenderPhase)
    {
        mCommandRecorder->Begin(CommandQueueType::Graphics);
    }

    // execute nodes in the group
    for (const RenderGraphNodePtr& node : mNodes)
    {
        if (isRenderPhase)
        {
            mCommandRecorder->BeginDebugGroup(node->GetName());
        }

        node->Execute(context);

        if (isRenderPhase)
        {
            mCommandRecorder->EndDebugGroup();
        }
    }

    if (isRenderPhase)
    {
        CommandListPtr commandList = mCommandRecorder->Finish();

        // TODO submission must be handled at the render graph level
        renderer.GetGraphicsQueue()->Execute(commandList);

        // TODO this is total hack
        if (frameInfo.viewport)
        {
            frameInfo.viewport->Present();
        }
    }
}

///

RenderGraph::RenderGraph()
{
}

RenderGraph::~RenderGraph()
{
}

void RenderGraph::Build(const FrameInfo& frameInfo)
{
    // TODO disable/enable nodes based on rendering features enabled

    mNodeGroups.Clear();

    RenderNodeGroupPtr mainGroup = MakeUniquePtr<RenderNodeGroup>();

    mainGroup->AddNode(MakeUniquePtr<RenderGraphNode_RenderScene>(frameInfo.scene));
    mainGroup->AddNode(MakeUniquePtr<RenderGraphNode_RenderImGui>());

    if (frameInfo.viewport)
    {
        mainGroup->AddNode(MakeUniquePtr<RenderGraphNode_Present>());
    }

    mNodeGroups.PushBack(std::move(mainGroup));

    // TODO link dependencies
}

// TODO Execute() shouldn't require frame info
void RenderGraph::Execute(const FrameInfo& frameInfo, RenderGraphResourceAllocator& resourceAllocator)
{
    resourceAllocator.Reset();

    // TODO can prepare phase be parallel?
    {
        for (const RenderNodeGroupPtr& group : mNodeGroups)
        {
            group->Execute(RenderGraphContext::Phase::Prepare, frameInfo, resourceAllocator);
        }
    }

    resourceAllocator.Resolve();

    // TODO parallel for
    {
        for (const RenderNodeGroupPtr& group : mNodeGroups)
        {
            group->Execute(RenderGraphContext::Phase::Render, frameInfo, resourceAllocator);
        }
    }
}

} // namespace Scene
} // namespace NFE
