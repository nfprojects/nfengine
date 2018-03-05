/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  RenderGraph class declaration
 */
#pragma once

#include "RenderGraphNode.hpp"
#include "RenderGraphPass.hpp"
#include "RenderGraphResource.hpp"


namespace NFE {
namespace Renderer {

class RenderGraph
{
    // collects root passes, which begin the graph
    Common::DynArray<RenderGraphPass::Ptr> mRootPasses;

    // debugging helpers
    void PrintNode(const RenderGraphNode::Ptr& ptr);
    void PrintPassNode(const RenderGraphNode::Ptr& node);

public:
    RenderGraph();
    ~RenderGraph();

    void AddRootPass(RenderGraphPass::Ptr& pass);

    // debugging helper
    void PrintGraph();

    static void AddInputBufferToPass(RenderGraphPass::Ptr& pass, RenderGraphBuffer::Ptr& buf);
    static void AddInputTextureToPass(RenderGraphPass::Ptr& pass, RenderGraphTexture::Ptr& tex);
    static void AddOutputBufferToPass(RenderGraphPass::Ptr& pass, RenderGraphBuffer::Ptr& buf);
    static void AddOutputTextureToPass(RenderGraphPass::Ptr& pass, RenderGraphTexture::Ptr& tex);
};

} // namespace Renderer
} // namespace NFE
