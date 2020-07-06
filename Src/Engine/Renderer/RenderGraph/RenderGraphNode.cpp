#include "PCH.hpp"
#include "RenderGraphNode.hpp"
#include "RenderGraphContext.hpp"
#include "RenderGraphResourceAllocator.hpp"
#include "../Viewport/Viewport.hpp"
#include "../../Renderers/RendererCommon/CommandRecorder.hpp"
#include "../../Common/Math/Vec4fU.hpp"


namespace NFE {
namespace Renderer {

IRenderGraphNode::~IRenderGraphNode() = default;

void RenderGraphNode_RenderScene::Execute(const RenderGraphContext& context) const
{
    TextureDesc colorTargetDesc;
    colorTargetDesc.width = context.viewport->GetWidth();
    colorTargetDesc.height = context.viewport->GetHeight();
    colorTargetDesc.format = Format::R16G16B16A16_Float; // TODO could be R11G11B10
    colorTargetDesc.mode = ResourceAccessMode::GPUOnly;
    colorTargetDesc.usage = TextureUsageFlag::RenderTarget;

    auto colorRenderTarget = context.resourceAllocator.Declare("color", colorTargetDesc);

    TextureDesc depthBufferDesc;
    depthBufferDesc.width = context.viewport->GetWidth();
    depthBufferDesc.height = context.viewport->GetHeight();
    depthBufferDesc.format = Format::Depth32;
    depthBufferDesc.mode = ResourceAccessMode::GPUOnly;
    depthBufferDesc.usage = TextureUsageFlag::DepthStencil;

    auto depthBuffer = context.resourceAllocator.Declare("depthBuffer", depthBufferDesc);

    if (context.IsRenderPhase())
    {
        const RenderTargetDesc renderTargetDesc =
        {
            { RenderTargetElement(colorRenderTarget) },
            depthBuffer
        };
        context.SetRenderTargets(renderTargetDesc);

        const uint32 slot = 0;
        const Math::Vec4fU color{ 0.1f, 0.2f, 0.3f, 1.0f };
        context.commandRecorder->Clear(ClearFlagsColor, 1, &slot, &color);
    }
}

void RenderGraphNode_RenderImGui::Execute(const RenderGraphContext& context) const
{
    auto colorRenderTarget = context.resourceAllocator.GetResource("color");

    if (context.IsRenderPhase())
    {
        // TODO
    }
}

void RenderGraphNode_Present::Execute(const RenderGraphContext& context) const
{
    auto colorRenderTarget = context.resourceAllocator.GetResource("color");

    if (context.IsRenderPhase())
    {
        context.commandRecorder->CopyTexture(colorRenderTarget, context.viewport->GetBackbuffer());
    }
}

} // namespace Scene
} // namespace NFE
