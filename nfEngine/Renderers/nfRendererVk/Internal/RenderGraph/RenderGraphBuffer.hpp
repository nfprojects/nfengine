/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declaration of Render Graph's Buffer node
 */
#pragma once

#include "RenderGraphResource.hpp"

#include <nfCommon/nfCommon.hpp>
#include <nfCommon/Containers/String.hpp>
#include <nfCommon/Containers/SharedPtr.hpp>


namespace NFE {
namespace Renderer {

class RenderGraphBuffer: public RenderGraphResource
{
public:
    using Ptr = Common::SharedPtr<RenderGraphBuffer>;

private:
    uint32_t mSize;

public:
    RenderGraphBuffer(const Common::String& name, uint32_t size);
    virtual ~RenderGraphBuffer();

    NFE_INLINE RenderGraphResource::Type GetResourceType() const override
    {
        return RenderGraphResource::Type::Buffer;
    }

    NFE_INLINE uint32_t GetSize() const
    {
        return mSize;
    }
};

} // namespace Renderer
} // namespace NFE
