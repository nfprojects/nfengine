/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Render Graph's Pass node
 */
#include "RenderGraphPass.hpp"


namespace NFE {
namespace Renderer {

RenderGraphPass::RenderGraphPass(const Common::String& name, uint32_t width, uint32_t height)
    : RenderGraphNode(name)
    , mWidth(width)
    , mHeight(height)
{
}

RenderGraphPass::~RenderGraphPass()
{
}

} // namespace Renderer
} // namespace NFE
