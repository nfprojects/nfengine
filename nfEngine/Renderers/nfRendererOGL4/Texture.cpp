/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Texture.hpp"
#include "Translations.hpp"


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

    glBindTexture(GL_TEXTURE_2D, mTexture);

    bool isNormalized;
    GLenum type = TranslateElementFormat(desc.format, isNormalized);

    // TODO support compressed formats: http://renderingpipeline.com/2012/07/texture-compression/
    GLenum format; // TODO provide support for GL_DEPTH_COMPONENT and GL_DEPTH_STENCIL
    switch (desc.texelSize)
    {
    case 1: format = GL_RED; break;
    case 2: format = GL_RG; break;
    case 3: format = GL_RGB; break;
    case 4: format = GL_RGBA; break;
    }

    // upload base level
    for (unsigned int i = 0; i < desc.mipmaps; ++i)
        glTexImage2D(GL_TEXTURE_2D, i, format, desc.width, desc.height, 0,
                     format, type, desc.dataDesc[i].data);

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
    // we can generate the texture here
    glGenTextures(1, &mTexture);

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
