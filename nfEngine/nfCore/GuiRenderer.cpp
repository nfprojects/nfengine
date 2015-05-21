/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level GUI Renderer
 */

#pragma once

#include "PCH.hpp"
#include "GuiRenderer.hpp"

namespace NFE {
namespace Renderer {

int Font::GetTextWidth(const char* pText)
{
    return 0;
}

void GuiRenderer::FlushQueue(NFE_CONTEXT_ARG)
{
}

void GuiRenderer::PushQuad(NFE_CONTEXT_ARG, const GuiQuad& quad)
{
}

void GuiRenderer::PushQuads(NFE_CONTEXT_ARG, const GuiQuad* pQuads, UINT size)
{
}

GuiRenderer::GuiRenderer()
{
}

GuiRenderer::~GuiRenderer()
{
}

Font* GuiRenderer::MakeFont(const char* pPath, int height)
{
    return nullptr;
}

void GuiRenderer::Enter(NFE_CONTEXT_ARG)
{
}

void GuiRenderer::Leave(NFE_CONTEXT_ARG)
{
}

void GuiRenderer::SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget)
{
}

void GuiRenderer::DrawQuad(NFE_CONTEXT_ARG, const Recti& rect, ITexture* pTexture,
                           Rectf* pTexCoords, UINT color, bool alpha)
{
}

void GuiRenderer::PrintText(NFE_CONTEXT_ARG, const char* pText, const Recti& Rect,
                            uint32 Align, Font* pFont, uint32 Color)
{
}

void GuiRenderer::PrintTextWithBorder(NFE_CONTEXT_ARG, const char* pText, const Recti& Rect,
                                      uint32 Align, Font* pFont, uint32 Color, uint32 BorderColor)
{
}

} // namespace Renderer
} // namespace NFE
