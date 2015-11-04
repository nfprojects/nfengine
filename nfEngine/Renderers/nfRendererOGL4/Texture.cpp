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
    : mType(TextureType::Unknown)
    , mWidth(0)
    , mHeight(0)
    , mTexelSize(0)
    , mTexture(GL_NONE)
    , mHasStencil(false)
{
}

Texture::~Texture()
{
    glDeleteTextures(1, &mTexture);
}

bool Texture::InitTexture1D(const TextureDesc& desc)
{
    UNUSED(desc);

    return true;
}

bool Texture::InitTexture2D(const TextureDesc& desc)
{
    // TODO support compressed formats: http://renderingpipeline.com/2012/07/texture-compression/
    bool isNormalized;
    GLenum type;
    GLenum format;
    GLenum internalFormat;

    if (desc.binding & (NFE_RENDERER_TEXTURE_BIND_SHADER | NFE_RENDERER_TEXTURE_BIND_RENDERTARGET))
    {
        type = TranslateElementFormatToType(desc.format, isNormalized);
        format = TranslateTexelSizeToFormat(desc.texelSize);
        internalFormat = format;
    }
    else if (desc.binding & NFE_RENDERER_TEXTURE_BIND_DEPTH)
    {
        type = TranslateDepthFormatToType(desc.depthBufferFormat);
        format = TranslateDepthFormatToFormat(desc.depthBufferFormat);
        internalFormat = TranslateDepthFormatToInternalFormat(desc.depthBufferFormat);

        if (desc.depthBufferFormat == DepthBufferFormat::Depth24_Stencil8)
            mHasStencil = true;
    }
    else
    {
        LOG_ERROR("Invalid texture binding flags.");
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, mTexture);

    // upload texture only if needed (for example Render Target might need an empty texture)
    const void* data;
    for (int i = 0; i < desc.mipmaps; ++i)
    {
        if (desc.dataDesc == nullptr)
            data = nullptr;
        else
            data = desc.dataDesc[i].data;

        glTexImage2D(GL_TEXTURE_2D, i, internalFormat, desc.width, desc.height, 0,
                     format, type, data);
    }

    // limit mipmap levels, otherwise no texture will be drawn
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, desc.mipmaps - 1);

    // Some basic filter settings to begin with.
    // Regular Textures will probably overwrite these settings with Sampler Objects, however
    // Depth Buffers will have use from them.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    mWidth = desc.width;
    mHeight = desc.height;

    return true;
}

bool Texture::InitTexture3D(const TextureDesc& desc)
{
    UNUSED(desc);

    return true;
}

bool Texture::Init(const TextureDesc& desc)
{
    // we can generate the texture here
    glGenTextures(1, &mTexture);
    mType = desc.type;

    switch (mType)
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
