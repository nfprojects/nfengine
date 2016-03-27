/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level GUI Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "../Resources/MultiShaderProgram.hpp"
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

class CORE_API GuiRenderer : public RendererModule<GuiRenderer>
{
    Resource::MultiShaderProgram mShaderProgram;
    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IBuffer> mConstantBuffer;
    std::unique_ptr<IPipelineState> mPipelineState;

    /// ImGui resources
    std::unique_ptr<IPipelineState> mImGuiPipelineState;
    Resource::MultiShaderProgram mImGuiShaderProgram;
    std::unique_ptr<IVertexLayout> mImGuiVertexLayout;
    std::unique_ptr<IBuffer> mImGuiVertexBuffer;
    std::unique_ptr<IBuffer> mImGuiIndexBuffer;

    void FlushQueue(RenderContext* context);
    void PushQuad(RenderContext* context, const GuiQuadData& quad,
                  const GuiQuadVertex& quadVertex);

public:
    GuiRenderer();

    void OnEnter(RenderContext* context);
    void OnLeave(RenderContext* context);
    void SetTarget(RenderContext* context, IRenderTarget* target);
    void BeginOrdinaryGuiRendering(RenderContext* context);

    void DrawQuad(RenderContext* context, const Rectf& rect, uint32 color);
    void DrawTexturedQuad(RenderContext* context, const Rectf& rect, const Rectf& texCoords,
                          ITexture* texture, uint32 color, bool alpha = false);
    bool PrintText(RenderContext* context, Font* font, const char* text,
                   const Recti& rect, uint32 color,
                   VerticalAlignment vAlign = VerticalAlignment::Top,
                   HorizontalAlignment hAlign = HorizontalAlignment::Right);
    bool PrintTextWithBorder(RenderContext* context, Font* font, const char* text,
                             const Recti& rect, uint32 color, uint32 borderColor,
                             VerticalAlignment vAlign = VerticalAlignment::Top,
                             HorizontalAlignment hAlign = HorizontalAlignment::Right);
    bool DrawImGui(RenderContext* context);
};

} // namespace Renderer
} // namespace NFE
