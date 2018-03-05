/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Render Graph's Operation node
 */
#include "PCH.hpp"
#include "RenderGraphOp.hpp"


namespace NFE {
namespace Renderer {

RenderGraphOp::RenderGraphOp(const Common::String& name, uint32_t width, uint32_t height)
    : RenderGraphNode(name)
    , mWidth(width)
    , mHeight(height)
{
}

RenderGraphOp::~RenderGraphOp()
{
}

} // namespace Renderer
} // namespace NFE
