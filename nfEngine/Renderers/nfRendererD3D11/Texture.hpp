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

class Texture : virtual public ITexture
{
    friend class CommandBuffer;
    friend class RenderTarget;
    friend class Device;
    friend class ResourceBindingInstance;

protected:
    TextureType mType;
    ElementFormat mFormat;
    int mWidth;
    int mHeight;
    int mLayers;
    int mMipmaps;
    int mSamples;
    D3DPtr<ID3D11ShaderResourceView> mSRV;
    D3DPtr<ID3D11DepthStencilView> mDSV;

    union
    {
        ID3D11Texture1D* mTexture1D;
        ID3D11Texture2D* mTexture2D;
        ID3D11Texture3D* mTexture3D;
        void* mTextureGeneric;
    };

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
