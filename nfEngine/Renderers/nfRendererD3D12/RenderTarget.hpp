/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's render target
 */

#pragma once

#include "../RendererInterface/RenderTarget.hpp"
#include "Common.hpp"
#include "Texture.hpp"

#include "nfCommon/System/Assertion.hpp"

#include <vector>


namespace NFE {
namespace Renderer {

class RenderTarget : public IRenderTarget
{
    struct Target
    {
        InternalTexturePtr texture;
        uint32 subresource;
    };

    // TODO: temporary
    std::vector<Target> mTargets;
    std::vector<uint32> mRTVs[2];

    InternalTexturePtr mDepthTexture;
    uint32 mDepthTextureSubresource;
    uint32 mDSV;

public:
    RenderTarget();
    ~RenderTarget();

    // IRenderTarget
    void GetDimensions(int& width, int& height);
    bool Init(const RenderTargetDesc& desc);


    NFE_INLINE size_t GetNumTargets() const
    {
        return mTargets.size();
    }

    // get texture pointer for given target ID
    NFE_INLINE const InternalTexturePtr& GetTexture(size_t targetID) const
    {
        NFE_ASSERT(targetID < mTargets.size(), "Invalid target ID");
        return mTargets[targetID].texture;
    }

    // get texture subresource index for given target ID
    NFE_INLINE uint32 GetSubresourceID(size_t targetID) const
    {
        NFE_ASSERT(targetID < mTargets.size(), "Invalid target ID");
        return mTargets[targetID].subresource;
    }

    // get RTV for given target ID
    NFE_INLINE uint32 GetRTV(size_t targetID) const
    {
        const InternalTexturePtr& tex = GetTexture(targetID);
        return mRTVs[tex->GetCurrentBuffer()][targetID];
    }

    // get depth texture pointer
    NFE_INLINE const InternalTexturePtr& GetDepthTexture() const
    {
        return mDepthTexture;
    }

    // get depth texture subresource index
    NFE_INLINE uint32 GetDepthTexSubresourceID() const
    {
        return mDepthTextureSubresource;
    }

    // get DSV
    NFE_INLINE uint32 GetDSV() const
    {
        return mDSV;
    }
};

} // namespace Renderer
} // namespace NFE
