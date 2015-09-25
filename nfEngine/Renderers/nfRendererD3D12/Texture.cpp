/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's texture
 */

#include "PCH.hpp"
#include "Texture.hpp"
#include "RendererD3D12.hpp"
#include "Translations.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Renderer {

Texture::Texture()
    : mBuffersNum(0)
    , mClass(Class::Regular)
    , mCurrentBuffer(0)
    , mResourceState(D3D12_RESOURCE_STATE_COMMON)
{
}

Texture::~Texture()
{
}

bool Texture::Init(const TextureDesc& desc)
{
    UNUSED(desc);
    return false;
}

} // namespace Renderer
} // namespace NFE
