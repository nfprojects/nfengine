/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declaration of Render Graph's Node
 */
#pragma once

#include <nfCommon/nfCommon.hpp>
#include <nfCommon/Containers/String.hpp>
#include <nfCommon/Containers/DynArray.hpp>
#include <nfCommon/Containers/SharedPtr.hpp>


namespace NFE {
namespace Renderer {

class RenderGraphNode
{
    friend class RenderGraph;

public:
    using Ptr = Common::SharedPtr<RenderGraphNode>;

    enum class Type: unsigned char
    {
        Unknown = 0,
        Pass,
        Resource,
    };

protected:
    Common::String mName;
    // TODO replace with Common::List when it is implemented
    Common::DynArray<RenderGraphNode::Ptr> mInputs;
    Common::DynArray<RenderGraphNode::Ptr> mOutputs;
    bool mTraverseFlag;

    NFE_INLINE void AddInput(RenderGraphNode::Ptr ptr)
    {
        mInputs.PushBack(ptr);
    }

    NFE_INLINE void AddOutput(RenderGraphNode::Ptr ptr)
    {
        mOutputs.PushBack(ptr);
    }

public:
    RenderGraphNode(const Common::String& name);
    virtual ~RenderGraphNode();

    virtual Type GetType() const = 0;
};

} // namespace Renderer
} // namespace NFE
