/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Render Graph's Buffer node
 */
#include "PCH.hpp"
#include "RenderGraphBuffer.hpp"


namespace NFE {
namespace Renderer {

RenderGraphBuffer::RenderGraphBuffer(const Common::String& name, uint32_t size)
    : RenderGraphResource(name)
    , mSize(size)
{
}

RenderGraphBuffer::~RenderGraphBuffer()
{
}

} // namespace Renderer
} // namespace NFE
