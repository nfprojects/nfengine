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
    friend class RenderTarget;
    friend class ResourceBindingInstance;

protected:

    // TODO this array is needed only for backbuffers
    D3DPtr<ID3D12Resource> mBuffers[2];

    uint32 mRowPitch;
    uint16 mWidth;
    uint16 mLayers;
    uint16 mHeight;
    uint16 mMipmapsNum;
    TextureType mType;
    BufferMode mMode;
    ElementFormat mFormat;
    DXGI_FORMAT mSrvFormat;
    DXGI_FORMAT mDsvFormat;

    uint32 mBuffersNum;
    uint32 mCurrentBuffer;

    D3D12_RESOURCE_STATES mTargetState; // TODO
    std::vector<D3D12_RESOURCE_STATES> mSubresourceStates;

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

    NFE_INLINE uint16 GetMipmapsNum() const
    {
        return mMipmapsNum;
    }

    NFE_INLINE D3D12_RESOURCE_STATES GetState(uint32 subresource) const
    {
        return mSubresourceStates[subresource];
    }

    NFE_INLINE D3D12_RESOURCE_STATES GetTargetState() const
    {
        return mTargetState;
    }

    NFE_INLINE void SetState(uint32 subresource, D3D12_RESOURCE_STATES newState)
    {
        mSubresourceStates[subresource] = newState;
    }

    uint32 GetCurrentBuffer() const
    {
        return mCurrentBuffer;
    }

    NFE_INLINE ID3D12Resource* GetResource() const
    {
        if (mBuffersNum == 1)
        {
            return mBuffers[0].get();
        }
        else
        {
            return mBuffers[mCurrentBuffer].get();
        }
    }

    NFE_INLINE bool IsBackbuffer() const
    {
        return mBuffersNum > 1;
    }

    NFE_INLINE BufferMode GetMode() const
    {
        return mMode;
    }

    NFE_INLINE ElementFormat GetFormat() const
    {
        return mFormat;
    }
};

} // namespace Renderer
} // namespace NFE
