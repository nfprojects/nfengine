/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Render Graph's Texture node
 */
#include "RenderGraphTexture.hpp"


namespace NFE {
namespace Renderer {

RenderGraphTexture::RenderGraphTexture(const Common::String& name, uint32_t width, uint32_t height)
    : RenderGraphResource(name)
    , mWidth(width)
    , mHeight(height)
{
}

RenderGraphTexture::~RenderGraphTexture()
{
}

} // namespace Renderer
} // namespace NFE
