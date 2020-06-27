/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's backbuffer.
 */

#pragma once

#include "../Interface/Texture.hpp"
#include "Common.hpp"
#include "Resource.hpp"

namespace NFE {
namespace Renderer {

class Texture : public Resource, public ITexture
{
    friend class RenderTarget;
    friend class ResourceBindingInstance;

protected:

    D3DPtr<ID3D12Resource> mResource;

    DXGI_FORMAT mSrvFormat;
    DXGI_FORMAT mDsvFormat;

    uint32 mRowPitch;
    uint16 mWidth;
    uint16 mLayers;
    uint16 mHeight;
    uint8 mMipmapsNum;
    uint8 mSamplesNum;
    TextureType mType;
    BufferMode mMode;
    Format mFormat;

    // upload initial data
    bool UploadData(const TextureDesc& desc);

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);

    // get row size (in bytes, for Readback textures only)
    NFE_INLINE uint32 GetRowPitch() const
    {
        return mRowPitch;
    }

    NFE_INLINE uint16 GetWidth() const
    {
        return mWidth;
    }

    NFE_INLINE uint16 GetHeight() const
    {
        return mHeight;
    }

    NFE_INLINE uint16 GetLayersNum() const
    {
        return mLayers;
    }

    NFE_INLINE uint8 GetMipmapsNum() const
    {
        return mMipmapsNum;
    }

    NFE_INLINE ID3D12Resource* GetResource() const
    {
        return mResource.Get();
    }

    NFE_INLINE BufferMode GetMode() const
    {
        return mMode;
    }

    NFE_INLINE Format GetFormat() const
    {
        return mFormat;
    }

    NFE_INLINE uint8 GetSamplesNum() const
    {
        return mSamplesNum;
    }
};

using InternalTexturePtr = Common::SharedPtr<Texture>;

} // namespace Renderer
} // namespace NFE
