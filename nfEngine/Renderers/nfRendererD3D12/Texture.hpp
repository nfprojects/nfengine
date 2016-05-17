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

    enum class Class : uint8
    {
        Regular,
        Backbuffer
    };

    // TODO this array is needed only for backbuffers
    D3DPtr<ID3D12Resource> mBuffers[2];

    uint16 mWidth;
    uint16 mLayers;
    uint16 mHeight;
    uint16 mMipmapsNum;
    uint16 mLayersNum;
    Class mClass;
    TextureType mType;
    DXGI_FORMAT mSrvFormat;

    uint32 mBuffersNum;
    uint32 mCurrentBuffer;

    D3D12_RESOURCE_STATES mResourceState;

    bool UploadData(const TextureDesc& desc);

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);
};

} // namespace Renderer
} // namespace NFE
