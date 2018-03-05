/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Render Graph's Resource node
 */
#include "PCH.hpp"
#include "RenderGraphResource.hpp"


namespace NFE {
namespace Renderer {

RenderGraphResource::RenderGraphResource(const Common::String& name)
    : RenderGraphNode(name)
{
}

RenderGraphResource::~RenderGraphResource()
{
}

} // namespace Renderer
} // namespace NFE
