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
    friend class ResourceBindingInstance;

protected:

    enum class Class
    {
        Regular,
        Backbuffer
    };

    Class mClass;
    TextureType mType;
    int mWidth;
    int mHeight;
    int mMipmapsNum;
    int mLayersNum;
    DXGI_FORMAT mSrvFormat;
    DXGI_FORMAT mDsvFormat;

    UINT mBuffersNum;
    UINT mCurrentBuffer;
    D3DPtr<ID3D12Resource> mBuffers[2];

    D3D12_RESOURCE_STATES mResourceState;
    D3D12_RESOURCE_STATES mTargetResourceState;

    bool UploadData(const TextureDesc& desc);

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);
};

} // namespace Renderer
} // namespace NFE
