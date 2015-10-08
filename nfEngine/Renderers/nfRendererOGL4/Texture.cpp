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
    , mGLType(GL_NONE)
    , mGLFormat(GL_NONE)
    , mGLInternalFormat(GL_NONE)
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

    if (desc.width == 0 || desc.height == 0)
    {
        LOG_ERROR("Invalid Texture dimensions");
        return false;
    }

    if (desc.access == BufferAccess::GPU_ReadOnly && desc.dataDesc == nullptr)
    {
        LOG_ERROR("Invalid data desc - when access is GPU_ReadOnly, there must be data to init!");
        return false;
    }

    if ((desc.binding & (NFE_RENDERER_TEXTURE_BIND_SHADER | NFE_RENDERER_TEXTURE_BIND_RENDERTARGET))
        || (desc.binding == 0))
    {
        mGLType = TranslateElementFormatToType(desc.format, isNormalized);
        mGLFormat = TranslateTexelSizeToFormat(desc.texelSize);
        mGLInternalFormat = TranslateFormatAndSizeToInternalFormat(desc.format, desc.texelSize);
    }
    else if (desc.binding & NFE_RENDERER_TEXTURE_BIND_DEPTH)
    {
        mGLType = TranslateDepthFormatToType(desc.depthBufferFormat);
        mGLFormat = TranslateDepthFormatToFormat(desc.depthBufferFormat);
        mGLInternalFormat = TranslateDepthFormatToInternalFormat(desc.depthBufferFormat);

        if (desc.depthBufferFormat == DepthBufferFormat::Depth24_Stencil8)
            mHasStencil = true;
    }
    else if (desc.binding)
    {
        LOG_ERROR("Invalid texture binding flags.");
        return false;
    }

    mWidth = desc.width;
    mHeight = desc.height;

    glBindTexture(GL_TEXTURE_2D, mTexture);

    if (desc.dataDesc == nullptr)
        // single-call immutable allocation (could be later changed by copy/TexSubImage)
        glTexStorage2D(GL_TEXTURE_2D, desc.mipmaps, mGLInternalFormat, mWidth, mHeight);
    else
    {
        // allocate all texture levels separately
        for (int i = 0; i < desc.mipmaps; ++i)
            glTexImage2D(GL_TEXTURE_2D, i, mGLInternalFormat, mWidth, mHeight, 0,
                         mGLFormat, mGLType, desc.dataDesc[i].data);
    }

    // limit mipmap levels, otherwise no texture will be drawn
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, desc.mipmaps - 1);

    // Some basic filter settings to begin with.
    // Regular Textures will probably overwrite these settings with Sampler Objects, however
    // Depth Buffers will have use from them.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
