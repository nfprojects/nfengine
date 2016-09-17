/**
 * @file
 * @author  Witek902
 * @brief   Definition of ReferencedResourcesList class
 */

#include "PCH.hpp"
#include "ReferencedResourcesList.hpp"

#include "../RendererInterface/Buffer.hpp"
#include "../RendererInterface/ComputePipelineState.hpp"
#include "../RendererInterface/PipelineState.hpp"
#include "../RendererInterface/Texture.hpp"
#include "../RendererInterface/RenderTarget.hpp"


namespace NFE {
namespace Renderer {

void ReferencedResourcesList::Clear()
{
    textures.Clear();
    buffers.Clear();
    renderTargets.Clear();
    bindingSetInstances.Clear();
    bindingSetLayouts.Clear();
    pipelineStates.Clear();
    computePipelineStates.Clear();
}

} // namespace Renderer
} // namespace NFE
