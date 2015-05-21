/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level GUI Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "GuiRenderer.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<GuiRenderer> GuiRenderer::mPtr;


int Font::GetTextWidth(const char* text)
{
    return 0;
}

void GuiRenderer::FlushQueue(RenderContext* context)
{
}

void GuiRenderer::PushQuad(RenderContext* context, const GuiQuad& quad)
{
}

void GuiRenderer::PushQuads(RenderContext* context, const GuiQuad* quads, UINT size)
{
}

Font* GuiRenderer::MakeFont(const char* pPath, int height)
{
    return nullptr;
}

void GuiRenderer::SetTarget(RenderContext* context, IRenderTarget* target)
{
}

void GuiRenderer::DrawQuad(RenderContext* context, const Recti& rect, ITexture* texture,
                           Rectf* texCoords, UINT color, bool alpha)
{
}

void GuiRenderer::PrintText(RenderContext* context, const char* text, const Recti& rect,
                            uint32 align, Font* font, uint32 color)
{
}

void GuiRenderer::PrintTextWithBorder(RenderContext* context, const char* text, const Recti& rect,
                                      uint32 align, Font* font, uint32 color, uint32 borderColor)
{
}

} // namespace Renderer
} // namespace NFE
