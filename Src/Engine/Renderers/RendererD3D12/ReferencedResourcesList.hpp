/**
 * @file
 * @author  Witek902
 * @brief   Declaration of ReferencedResourcesList class
 */

#pragma once

#include "../RendererCommon/Types.hpp"
#include "Engine/Common/Containers/HashSet.hpp"


namespace NFE {
namespace Renderer {

 // used for tracking resources references (Direct3D 12 does not do it by its own).
struct ReferencedResourcesList
{
    template<typename T>
    using Container = Common::HashSet<T>;

    Container<TexturePtr> textures;
    Container<BufferPtr> buffers;
    Container<RenderTargetPtr> renderTargets;
    Container<PipelineStatePtr> pipelineStates;
    Container<ComputePipelineStatePtr> computePipelineStates;

    ReferencedResourcesList();
    ReferencedResourcesList(ReferencedResourcesList&&);
    ReferencedResourcesList& operator = (ReferencedResourcesList&&);
    ~ReferencedResourcesList();

    // delete, because they are costly
    ReferencedResourcesList(const ReferencedResourcesList&) = delete;
    ReferencedResourcesList& operator = (const ReferencedResourcesList&) = delete;

    void Clear();

    // add elements from other list to this list
    void MergeWith(const ReferencedResourcesList& other);
};

} // namespace Renderer
} // namespace NFE
