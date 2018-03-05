/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  RenderGraph class implementation
 */
#include "PCH.hpp"
#include "RenderGraph.hpp"

#include <nfCommon/Logger/Logger.hpp>
#include <nfCommon/Containers/DynArray.hpp>


namespace NFE {
namespace Renderer {


RenderGraph::RenderGraph()
{
}

RenderGraph::~RenderGraph()
{
}

void RenderGraph::PrintNode(const RenderGraphNode::Ptr& node)
{
    NFE_LOG_DEBUG("    Node name: %s", node->mName.Str());
    switch (node->GetType())
    {
    case RenderGraphNode::Type::Resource:
    {
        const RenderGraphResource* res = dynamic_cast<const RenderGraphResource*>(node.Get());

        NFE_LOG_DEBUG("      Type: Resource");
        switch (res->GetResourceType())
        {
        case RenderGraphResource::Type::Buffer:
        {
            const RenderGraphBuffer* buffer = dynamic_cast<const RenderGraphBuffer*>(res);
            NFE_LOG_DEBUG("      Resource Type: Buffer");
            NFE_LOG_DEBUG("        Size: %u", buffer->GetSize());
            break;
        }
        case RenderGraphResource::Type::Texture:
        {
            const RenderGraphTexture* tex = dynamic_cast<const RenderGraphTexture*>(res);
            NFE_LOG_DEBUG("      Resource Type: Texture");
            NFE_LOG_DEBUG("        Width:  %u", tex->GetWidth());
            NFE_LOG_DEBUG("        Height: %u", tex->GetHeight());
            break;
        }
        default:
        {
            NFE_LOG_DEBUG("      Resource Type: Unknown");
        }
        }

        break;
    }
    default:
    {
        NFE_LOG_DEBUG("      Type: Unknown");
    }
    }
}

void RenderGraph::PrintPassNode(const RenderGraphNode::Ptr& node)
{
    if (node->GetType() != RenderGraphNode::Type::Op)
        return;

    const RenderGraphOp* pass = dynamic_cast<const RenderGraphOp*>(node.Get());

    NFE_LOG_DEBUG("Render Graph operation node %s:", node->mName.Str());
    NFE_LOG_DEBUG("  Width:  %u", pass->GetWidth());
    NFE_LOG_DEBUG("  Height: %u", pass->GetHeight());

    NFE_LOG_DEBUG("  Inputs:");
    for (auto& n: pass->mInputs)
    {
        PrintNode(n);
    }

    NFE_LOG_DEBUG("  Outputs:");
    for (auto& n: pass->mOutputs)
    {
        PrintNode(n);
    }
}

void RenderGraph::AddRootNode(RenderGraphNode::Ptr& node)
{
    mRootNodes.PushBack(node);
}

void RenderGraph::PrintGraph()
{
    // TODO change to Queue when it is implemented in nfCommon
    Common::DynArray<RenderGraphNode::Ptr> nodes;

    for (auto& rn: mRootNodes)
    {
        nodes.PushBack(rn);
        rn->mTraverseFlag ^= true;
    }

    if (nodes.Empty())
        return;

    uint32_t queueTail = 0;
    bool currentTraverseFlag = !nodes[queueTail]->mTraverseFlag;

    while (queueTail < nodes.Size())
    {
        RenderGraphNode::Ptr node = nodes.Front();

        for (auto& n: node->mOutputs)
        {
            if (n->mTraverseFlag == currentTraverseFlag)
            {
                nodes.PushBack(n);
                n->mTraverseFlag ^= true;
            }
        }

        PrintPassNode(node);

        queueTail++;
    }
}

} // namespace Renderer
} // namespace NFE
