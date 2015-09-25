/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's texture
 */

#include "PCH.hpp"
#include "Texture.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"
#include "../../nfCommon/Logger.hpp"

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
    switch (desc.type)
    {
    case TextureType::Texture1D:
        return InitTexture1D(desc);
    case TextureType::Texture2D:
    case TextureType::TextureCube:
        return InitTexture2D(desc);
    case TextureType::Texture3D:
        return InitTexture3D(desc);
    }

    return false;
}

} // namespace Renderer
} // namespace NFE
