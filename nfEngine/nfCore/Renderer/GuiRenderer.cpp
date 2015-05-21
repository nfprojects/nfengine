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


int Font::GetTextWidth(const char* pText)
{
    return 0;
}

void GuiRenderer::FlushQueue(RenderContext *pContext)
{
}

void GuiRenderer::PushQuad(RenderContext *pContext, const GuiQuad& quad)
{
}

void GuiRenderer::PushQuads(RenderContext *pContext, const GuiQuad* pQuads, UINT size)
{
}

Font* GuiRenderer::MakeFont(const char* pPath, int height)
{
    return nullptr;
}

void GuiRenderer::SetTarget(RenderContext *pContext, IRenderTarget* pTarget)
{
}

void GuiRenderer::DrawQuad(RenderContext *pContext, const Recti& rect, ITexture* pTexture,
                           Rectf* pTexCoords, UINT color, bool alpha)
{
}

void GuiRenderer::PrintText(RenderContext *pContext, const char* pText, const Recti& Rect,
                            uint32 Align, Font* pFont, uint32 Color)
{
}

void GuiRenderer::PrintTextWithBorder(RenderContext *pContext, const char* pText, const Recti& Rect,
                                      uint32 Align, Font* pFont, uint32 Color, uint32 BorderColor)
{
}

} // namespace Renderer
} // namespace NFE
