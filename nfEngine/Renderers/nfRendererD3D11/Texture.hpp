/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 renderer's backbuffer.
 */

#pragma once

#include "../RendererInterface/Texture.hpp"
#include "Common.hpp"

namespace NFE {
namespace Renderer {

class Texture : virtual public TexturePtr
{
    friend class CommandRecorder;
    friend class RenderTarget;
    friend class Device;
    friend class ResourceBindingInstance;

protected:

    union
    {
        ID3D11Texture1D* mTexture1D;
        ID3D11Texture2D* mTexture2D;
        ID3D11Texture3D* mTexture3D;

        ID3D11Resource* mTextureGeneric;
    };

    D3DPtr<ID3D11DepthStencilView> mDSV;

    TextureType mType;
    ElementFormat mFormat;
    uint16 mWidth;
    uint16 mLayers;
    uint16 mHeight;
    uint16 mMipmaps;
    uint16 mSamples;

    DXGI_FORMAT mSrvFormat;


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
