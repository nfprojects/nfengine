/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Texture.hpp"


namespace NFE {
namespace Renderer {

Texture::Texture()
{
}

Texture::~Texture()
{
}

bool Init(const TextureDesc& desc)
{
    UNUSED(desc);
    return true;
}

} // namespace Renderer
} // namespace NFE
