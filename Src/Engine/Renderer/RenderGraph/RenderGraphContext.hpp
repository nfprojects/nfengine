#pragma once

#include "../RendererApi.hpp"
#include "../../Renderers/RendererCommon/Texture.hpp"
#include "../../Renderers/RendererCommon/Buffer.hpp"
#include "../../Renderers/RendererCommon/RenderTarget.hpp"


namespace NFE {
namespace Renderer {

class RenderGraphContext
{
public:
    enum class Phase : uint8
    {
        Prepare,
        Render,
    };

    Phase phase;
    RenderGraphResourceAllocator& resourceAllocator;
    const IViewport* viewport = nullptr;    // TODO this is wrong, there could be multiple viewports in one graph
    ICommandRecorder* commandRecorder = nullptr;

    bool IsRenderPhase() const { return phase == Phase::Render; }

    void SetRenderTargets(const RenderTargetDesc& desc) const;
};

} // namespace Renderer
} // namespace NFE
