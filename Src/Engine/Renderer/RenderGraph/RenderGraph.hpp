#pragma once

#include "RenderGraphContext.hpp"


namespace NFE {
namespace Renderer {

class IRenderGraphNode;
using RenderGraphNodePtr = Common::UniquePtr<IRenderGraphNode>;

class RenderNodeGroup;
using RenderNodeGroupPtr = Common::UniquePtr<RenderNodeGroup>;

// TODO move to separate file
// TODO command list dependencies (CPU and GPU)
// TODO groups without command list (CPU only)
class RenderNodeGroup
{
public:
    RenderNodeGroup();
    ~RenderNodeGroup();

    void AddNode(RenderGraphNodePtr&& node);
    void Execute(RenderGraphContext::Phase phase, const FrameInfo& frameInfo, RenderGraphResourceAllocator& allocator);

private:
    CommandRecorderPtr mCommandRecorder;
    Common::DynArray<RenderGraphNodePtr> mNodes;
};

class RenderGraph
{
public:
    RenderGraph();
    ~RenderGraph();

    void Build(const FrameInfo& frameInfo);
    void Execute(const FrameInfo& frameInfo, RenderGraphResourceAllocator& resourceAllocator);

private:
    Common::DynArray<RenderNodeGroupPtr> mNodeGroups;
};


} // namespace Renderer
} // namespace NFE
