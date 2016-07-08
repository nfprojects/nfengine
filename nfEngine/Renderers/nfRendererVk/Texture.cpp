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
{
}

Texture::~Texture()
{
}

bool Texture::InitTexture1D(const TextureDesc& desc)
{
    UNUSED(desc);
    return false;
}

bool Texture::InitTexture2D(const TextureDesc& desc)
{
    UNUSED(desc);
    return false;
}

bool Texture::InitTexture3D(const TextureDesc& desc)
{
    UNUSED(desc);
    return false;
}

bool Texture::Init(const TextureDesc& desc)
{
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
