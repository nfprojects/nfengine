/**
 * @file
 * @author  Witek902
 * @brief   Definition of ReferencedResourcesList class
 */

#include "PCH.hpp"
#include "ReferencedResourcesList.hpp"

#include "../RendererCommon/Buffer.hpp"
#include "../RendererCommon/ComputePipelineState.hpp"
#include "../RendererCommon/PipelineState.hpp"
#include "../RendererCommon/Texture.hpp"
#include "../RendererCommon/RenderTarget.hpp"
#include "../RendererCommon/Backbuffer.hpp"


namespace NFE {
namespace Renderer {


ReferencedResourcesList::ReferencedResourcesList() = default;
ReferencedResourcesList::ReferencedResourcesList(ReferencedResourcesList&&) = default;
ReferencedResourcesList& ReferencedResourcesList::operator = (ReferencedResourcesList&&) = default;
ReferencedResourcesList::~ReferencedResourcesList() = default;

void ReferencedResourcesList::Clear()
{
    backbuffers.Clear();
    textures.Clear();
    buffers.Clear();
    renderTargets.Clear();
    bindingSetInstances.Clear();
    bindingSetLayouts.Clear();
    pipelineStates.Clear();
    computePipelineStates.Clear();
}

void ReferencedResourcesList::MergeWith(const ReferencedResourcesList& other)
{
    for (const BackbufferPtr& backbuffer : other.backbuffers)
    {
        backbuffers.Insert(backbuffer);
    }

    for (const TexturePtr& texture : other.textures)
    {
        textures.Insert(texture);
    }

    for (const BufferPtr& buffer : other.buffers)
    {
        buffers.Insert(buffer);
    }

    for (const RenderTargetPtr& rt : other.renderTargets)
    {
        renderTargets.Insert(rt);
    }

    for (const ResourceBindingInstancePtr& bsi : other.bindingSetInstances)
    {
        bindingSetInstances.Insert(bsi);
    }

    for (const ResourceBindingLayoutPtr& bsl : other.bindingSetLayouts)
    {
        bindingSetLayouts.Insert(bsl);
    }

    for (const PipelineStatePtr& pipelineState : other.pipelineStates)
    {
        pipelineStates.Insert(pipelineState);
    }

    for (const ComputePipelineStatePtr& pipelineState : other.computePipelineStates)
    {
        computePipelineStates.Insert(pipelineState);
    }
}


} // namespace Renderer
} // namespace NFE
