/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#pragma once

#include "../RendererInterface/Texture.hpp"
#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class Texture : virtual public ITexture
{
    friend class CommandBuffer;
    friend class RenderTarget;

protected:
    TextureType type;
    int mWidth;
    int mHeight;
    int mTexelSize; //< texel size in bytes
    GLuint mTexture;

    bool InitTexture1D(const TextureDesc& desc);
    bool InitTexture2D(const TextureDesc& desc);
	bool InitTexture3D(const TextureDesc& desc);

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);
};

} // namespace Renderer
} // namespace NFE
