/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declaration of Render Graph's Resource node
 */
#pragma once

#include "RenderGraphNode.hpp"

#include <nfCommon/nfCommon.hpp>
#include <nfCommon/Containers/String.hpp>
#include <nfCommon/Containers/SharedPtr.hpp>


namespace NFE {
namespace Renderer {

class RenderGraphResource: public RenderGraphNode
{
    friend class RenderGraph;

public:
    using Ptr = Common::SharedPtr<RenderGraphResource>;

    enum class Type: unsigned char
    {
        Unknown = 0,
        Buffer,
        Texture,
    };

    RenderGraphResource(const Common::String& name);
    virtual ~RenderGraphResource();

    virtual Type GetResourceType() const = 0;

    NFE_INLINE RenderGraphNode::Type GetType() const override
    {
        return RenderGraphNode::Type::Resource;
    }

    NFE_INLINE void AddSourcePass(RenderGraphNode::Ptr ptr)
    {
        AddInput(ptr);
    }

    NFE_INLINE void AddDestinationPass(RenderGraphNode::Ptr ptr)
    {
        AddOutput(ptr);
    }
};

} // namespace Renderer
} // namespace NFE
