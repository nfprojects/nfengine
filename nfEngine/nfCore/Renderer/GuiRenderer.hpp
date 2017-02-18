/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level GUI Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "../Resources/MultiPipelineState.hpp"
#include "GuiRendererContext.hpp"
#include "Font.hpp"

namespace NFE {
namespace Renderer {

enum class VerticalAlignment
{
    Top,
    Center,
    Bottom,
};

enum class HorizontalAlignment
{
    Left,
    Right,
    Center,
    Justify
};

class CORE_API GuiRenderer : public RendererModule<GuiRenderer, GuiRendererContext>
{
    Resource::MultiPipelineState mPipelineState;
    BufferPtr mVertexBuffer;
    VertexLayoutPtr mVertexLayout;
    BufferPtr mConstantBuffer;

    ResourceBindingSetPtr mCBufferBindingSet;
    ResourceBindingSetPtr mPSBindingSet;
    ResourceBindingLayoutPtr mResBindingLayout;

    /// ImGui resources
    Resource::MultiPipelineState mImGuiPipelineState;
    VertexLayoutPtr mImGuiVertexLayout;
    BufferPtr mImGuiVertexBuffer;
    BufferPtr mImGuiIndexBuffer;

    bool CreateResourceBindingLayouts();
    void FlushQueue(GuiRendererContext* context);
    void PushQuad(GuiRendererContext* context, const GuiQuadData& quad,
                  const GuiQuadVertex& quadVertex);

public:
    GuiRenderer();

    ResourceBindingInstancePtr CreateTextureBinding(const TexturePtr& texture);

    void OnEnter(GuiRendererContext* context) override;
    void OnLeave(GuiRendererContext* context) override;

    void SetTarget(GuiRendererContext* context, RenderTargetPtr target);
    void BeginOrdinaryGuiRendering(GuiRendererContext* context);

    void DrawQuad(GuiRendererContext* context, const Rectf& rect, uint32 color);
    void DrawTexturedQuad(GuiRendererContext* context, const Rectf& rect, const Rectf& texCoords,
                          ResourceBindingInstancePtr textureBinding, uint32 color,
                          bool alpha = false);
    bool PrintText(GuiRendererContext* context, Font* font, const char* text,
                   const Recti& rect, uint32 color,
                   VerticalAlignment vAlign = VerticalAlignment::Top,
                   HorizontalAlignment hAlign = HorizontalAlignment::Right);
    bool PrintTextWithBorder(GuiRendererContext* context, Font* font, const char* text,
                             const Recti& rect, uint32 color, uint32 borderColor,
                             VerticalAlignment vAlign = VerticalAlignment::Top,
                             HorizontalAlignment hAlign = HorizontalAlignment::Right);
    bool DrawImGui(GuiRendererContext* context, ResourceBindingInstancePtr imGuiTextureBinding);
};

} // namespace Renderer
} // namespace NFE
