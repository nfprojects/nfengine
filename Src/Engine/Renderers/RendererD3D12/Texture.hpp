/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's backbuffer.
 */

#pragma once

#include "../RendererCommon/Texture.hpp"
#include "Common.hpp"
#include "Resource.hpp"

namespace NFE {
namespace Renderer {

class Texture : public Resource, public ITexture
{
    friend class RenderTarget;
    friend class ResourceBindingInstance;

protected:

    DXGI_FORMAT mSrvFormat;
    DXGI_FORMAT mDsvFormat;

    uint32 mRowPitch;
    uint16 mWidth;
    uint16 mHeight;
    uint16 mLayersNumOrDepth;
    uint8 mMipmapsNum : 4;
    uint8 mSamplesNum : 4;
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

    NFE_INLINE uint16 GetDepth() const
    {
        return mType == TextureType::Texture3D ? mLayersNumOrDepth : 1;
    }

    NFE_INLINE uint16 GetLayersNum() const
    {
        return mType == TextureType::Texture3D ? 1 : mLayersNumOrDepth;
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

    NFE_INLINE TextureType GetType() const
    {
        return mType;
    }
};

using InternalTexturePtr = Common::SharedPtr<Texture>;

} // namespace Renderer
} // namespace NFE
