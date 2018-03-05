/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declaration of Render Graph's Texture node
 */
#pragma once

#include "RenderGraphResource.hpp"


namespace NFE {
namespace Renderer {

class RenderGraphTexture: public RenderGraphResource
{
public:
    using Ptr = Common::SharedPtr<RenderGraphTexture>;

private:
    uint32_t mWidth;
    uint32_t mHeight;

public:
    RenderGraphTexture(const Common::String& name, uint32_t width, uint32_t height);
    virtual ~RenderGraphTexture();

    NFE_INLINE RenderGraphResource::Type GetResourceType() const override
    {
        return RenderGraphResource::Type::Texture;
    }

    NFE_INLINE uint32_t GetWidth() const
    {
        return mWidth;
    }

    NFE_INLINE uint32_t GetHeight() const
    {
        return mHeight;
    }
};

} // namespace Renderer
} // namespace NFE
