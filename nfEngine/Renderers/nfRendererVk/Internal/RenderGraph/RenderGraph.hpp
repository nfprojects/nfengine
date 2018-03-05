/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  RenderGraph class declaration
 */
#pragma once

#include "RenderGraphNode.hpp"
#include "RenderGraphOp.hpp"
#include "RenderGraphResource.hpp"


namespace NFE {
namespace Renderer {

class RenderGraph
{
    // collects root nodes, which begin the graph
    Common::DynArray<RenderGraphNode::Ptr> mRootNodes;

    // debugging helpers
    void PrintNode(const RenderGraphNode::Ptr& ptr);
    void PrintPassNode(const RenderGraphNode::Ptr& node);

public:
    RenderGraph();
    ~RenderGraph();

    void AddRootNode(RenderGraphNode::Ptr& node);

    // debugging helper
    void PrintGraph();
};

} // namespace Renderer
} // namespace NFE
