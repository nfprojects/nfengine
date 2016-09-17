/**
 * @file
 * @author  Witek902
 * @brief   Declaration of ReferencedResourcesList class
 */

#pragma once

#include "../RendererInterface/Types.hpp"
#include "nfCommon/Containers/HashSet.hpp"


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
    Container<ResourceBindingInstancePtr> bindingSetInstances;
    Container<ResourceBindingLayoutPtr> bindingSetLayouts;
    Container<PipelineStatePtr> pipelineStates;
    Container<ComputePipelineStatePtr> computePipelineStates;

    void Clear();
};

} // namespace Renderer
} // namespace NFE
