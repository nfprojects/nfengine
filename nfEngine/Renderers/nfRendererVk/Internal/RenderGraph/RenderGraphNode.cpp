/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Render Graph's Node
 */
#include "PCH.hpp"
#include "RenderGraphNode.hpp"


namespace NFE {
namespace Renderer {

RenderGraphNode::RenderGraphNode(const Common::String& name)
    : mName(name)
    , mTraverseFlag(false)
{
}

RenderGraphNode::~RenderGraphNode()
{
}

} // namespace Renderer
} // namespace NFE
