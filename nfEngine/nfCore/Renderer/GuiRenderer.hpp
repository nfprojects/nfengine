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
    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IBuffer> mConstantBuffer;

    std::unique_ptr<IResourceBindingSet> mCBufferBindingSet;
    std::unique_ptr<IResourceBindingSet> mPSBindingSet;
    std::unique_ptr<IResourceBindingLayout> mResBindingLayout;

    /// ImGui resources
    Resource::MultiPipelineState mImGuiPipelineState;
    std::unique_ptr<IVertexLayout> mImGuiVertexLayout;
    std::unique_ptr<IBuffer> mImGuiVertexBuffer;
    std::unique_ptr<IBuffer> mImGuiIndexBuffer;

    bool CreateResourceBindingLayouts();
    void FlushQueue(GuiRendererContext* context);
    void PushQuad(GuiRendererContext* context, const GuiQuadData& quad,
                  const GuiQuadVertex& quadVertex);

public:
    GuiRenderer();

    std::unique_ptr<IResourceBindingInstance> CreateTextureBinding(ITexture* texture);

    void OnEnter(GuiRendererContext* context) override;
    void OnLeave(GuiRendererContext* context) override;

    void SetTarget(GuiRendererContext* context, IRenderTarget* target);
    void BeginOrdinaryGuiRendering(GuiRendererContext* context);

    void DrawQuad(GuiRendererContext* context, const Rectf& rect, uint32 color);
    void DrawTexturedQuad(GuiRendererContext* context, const Rectf& rect, const Rectf& texCoords,
                          IResourceBindingInstance* textureBinding, uint32 color,
                          bool alpha = false);
    bool PrintText(GuiRendererContext* context, Font* font, const char* text,
                   const Recti& rect, uint32 color,
                   VerticalAlignment vAlign = VerticalAlignment::Top,
                   HorizontalAlignment hAlign = HorizontalAlignment::Right);
    bool PrintTextWithBorder(GuiRendererContext* context, Font* font, const char* text,
                             const Recti& rect, uint32 color, uint32 borderColor,
                             VerticalAlignment vAlign = VerticalAlignment::Top,
                             HorizontalAlignment hAlign = HorizontalAlignment::Right);
    bool DrawImGui(GuiRendererContext* context, IResourceBindingInstance* imGuiTextureBinding);
};

} // namespace Renderer
} // namespace NFE
