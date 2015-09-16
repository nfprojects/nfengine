/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level GUI Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "Multishader.hpp"
#include "GuiRendererContext.hpp"

namespace NFE {
namespace Renderer {

class CORE_API GuiRenderer : public RendererModule<GuiRenderer>
{
    Multishader mVertexShader;
    Multishader mGeometryShader;
    Multishader mPixelShader;

    std::unique_ptr<IBuffer> mVertexBuffer;
    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IBuffer> mConstantBuffer;
    std::unique_ptr<IBlendState> mBlendState;

    void FlushQueue(RenderContext* context);
    void PushQuad(RenderContext* context, const GuiQuadData& quad,
                  const GuiQuadVertex& quadVertex);

public:
    GuiRenderer();

    void OnEnter(RenderContext* context);
    void OnLeave(RenderContext* context);
    void SetTarget(RenderContext* context, IRenderTarget* target);

    void DrawQuad(RenderContext* context, const Rectf& rect, uint32 color);
    void DrawTexturedQuad(RenderContext* context, const Rectf& rect, const Rectf& texCoords,
                          ITexture* texture, uint32 color, bool alpha = false);
};

} // namespace Renderer
} // namespace NFE
