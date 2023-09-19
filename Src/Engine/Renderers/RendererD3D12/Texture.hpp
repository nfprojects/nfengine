/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 renderer's backbuffer.
 */

#pragma once

#include "../RendererCommon/Texture.hpp"

#include "Descriptors.hpp"
#include "Common.hpp"
#include "Resource.hpp"

namespace NFE {
namespace Renderer {

class Texture : public Resource, public ITexture
{
    friend class RenderTarget;
    friend class ResourceBindingInstance;

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);

    // get or create descriptor of a texture view
    [[nodiscard]] DescriptorID GetDescriptor(DescriptorType type, const TextureView& view);

    NFE_FORCE_INLINE uint32 GetRowPitch() const { return mRowPitch; }
    NFE_FORCE_INLINE uint16 GetWidth() const { return mWidth; }
    NFE_FORCE_INLINE uint16 GetHeight() const { return mHeight; }
    NFE_FORCE_INLINE uint16 GetDepth() const { return mType == TextureType::Texture3D ? mLayersNumOrDepth : 1; }
    NFE_FORCE_INLINE uint16 GetLayersNum() const { return mType == TextureType::Texture3D ? 1 : mLayersNumOrDepth; }
    NFE_FORCE_INLINE uint8 GetMipmapsNum() const { return mMipmapsNum; }
    NFE_FORCE_INLINE ID3D12Resource* GetResource() const { return mResource.Get(); }
    NFE_FORCE_INLINE Format GetFormat() const { return mFormat; }
    NFE_FORCE_INLINE uint8 GetSamplesNum() const { return mSamplesNum; }
    NFE_FORCE_INLINE TextureType GetType() const { return mType; }
    NFE_FORCE_INLINE DXGI_FORMAT GetSrvFormat() const { return mSrvFormat; }

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
    Format mFormat;

    // TODO this wastes a lot of space...
    // maybe introduce "default" descriptor created when buffer is created?
    struct DescriptorInfo
    {
        DescriptorType type;
        TextureView view;
        DescriptorID descriptor = UINT32_MAX;
    };
    Common::DynArray<DescriptorInfo> mDescriptors;

    void DeleteDescriptors();
};

using InternalTexturePtr = Common::SharedPtr<Texture>;

} // namespace Renderer
} // namespace NFE
