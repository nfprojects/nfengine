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

bool Texture::InitTexture1D(const TextureDesc& desc)
{
    UNUSED(desc);

    type = TextureType::Texture1D;
    return true;
}

bool Texture::InitTexture2D(const TextureDesc& desc)
{
    if (desc.binding & ~(NFE_RENDERER_TEXTURE_BIND_SHADER |
                         NFE_RENDERER_TEXTURE_BIND_RENDERTARGET |
                         NFE_RENDERER_TEXTURE_BIND_DEPTH))
    {
        LOG_ERROR("Invalid texture binding flags.");
        return false;
    }



    type = TextureType::Texture2D;
    return true;
}

bool Texture::InitTexture3D(const TextureDesc& desc)
{
    UNUSED(desc);

    type = TextureType::Texture3D;
    return true;
}

bool Texture::Init(const TextureDesc& desc)
{
    switch (desc.type)
    {
    case TextureType::Texture1D:
        return InitTexture1D(desc);
    case TextureType::Texture2D:
        return InitTexture2D(desc);
    case TextureType::Texture3D:
        return InitTexture3D(desc);
    default:
        LOG_ERROR("Unsupported or incorrect texture type.");
        return false;
    }
}

} // namespace Renderer
} // namespace NFE
