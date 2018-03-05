/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declaration of Render Graph's Pass node
 */
#pragma once

#include "RenderGraphResource.hpp"
#include "RenderGraphBuffer.hpp"
#include "RenderGraphTexture.hpp"

#include <nfCommon/nfCommon.hpp>
#include <nfCommon/Containers/String.hpp>
#include <nfCommon/Containers/SharedPtr.hpp>


namespace NFE {
namespace Renderer {

class RenderGraphPass: public RenderGraphNode
{
    friend class RenderGraph;

public:
    using Ptr = Common::SharedPtr<RenderGraphPass>;

private:
    uint32_t mWidth;
    uint32_t mHeight;

public:
    RenderGraphPass(const Common::String& name, uint32_t width, uint32_t height);
    virtual ~RenderGraphPass();

    NFE_INLINE RenderGraphNode::Type GetType() const override
    {
        return RenderGraphNode::Type::Pass;
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
