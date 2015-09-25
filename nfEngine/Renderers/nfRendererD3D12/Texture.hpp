/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's backbuffer.
 */

#pragma once

#include "../RendererInterface/Texture.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class Texture : virtual public ITexture
{
    friend class CommandBuffer;
    friend class RenderTarget;

protected:

    enum class Class
    {
        Regular,
        Backbuffer
    };

    Class mClass;
    TextureType mType;

    UINT mBuffersNum;
    UINT mCurrentBuffer;
    D3DPtr<ID3D12Resource> mBuffers[2];

    D3D12_RESOURCE_STATES mResourceState;

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
