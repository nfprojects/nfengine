/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level GUI Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"

namespace NFE {
namespace Renderer {

// text alignment
#define NFE_TEXT_ALIGN_X_LEFT (1 << 0)
#define NFE_TEXT_ALIGN_X_CENTER (0)
#define NFE_TEXT_ALIGN_X_RIGHT (1 << 1)
#define NFE_TEXT_ALIGN_Y_TOP (1 << 2)
#define NFE_TEXT_ALIGN_Y_CENTER (0)
#define NFE_TEXT_ALIGN_Y_BOTTOM (1 << 3)

struct CharacterInfo
{
    bool exists;
    short width, height;
    short spacing;
    short top, left, u, v;
    unsigned int glyphIndex;
};

class Font
{
public:
    ITexture* texture;
    CharacterInfo* characters;
    int height;
    int charCount;
    int texWidth, texHeight;

    //bool Build(const char* pFileName);
    //void Write(const wchar_t* pText, int Count, DWORD Color, UINT Align, uiRect* pRect, uiRect* pCropRect);
    int GetTextWidth(const char* pText);
    //int GetLength(const wchar_t* pText, int Count);
};

struct GuiQuad
{
    Recti rect;
    Rectf texCoord;
    ITexture* pTexture;
    UINT color;
    bool alphaTexture;

};

struct GuiVertex
{
    Recti rect;
    Rectf texCoord;
    uint32 color;
};

class GuiRenderer : public RendererModule<GuiRenderer>
{
    // FT_Library mFreeTypeLibrary;

    void FlushQueue(RenderContext* context);
    void PushQuad(RenderContext* context, const GuiQuad& quad);
    void PushQuads(RenderContext* context, const GuiQuad* quads, UINT size);

public:
    Font* MakeFont(const char* pPath, int height);

    void SetTarget(RenderContext* context, IRenderTarget* target);
    void DrawQuad(RenderContext* context, const Recti& rect, ITexture* texture, Rectf* texCoords,
                  UINT color, bool alpha = false);
    void PrintText(RenderContext* context, const char* text, const Recti& rect, uint32 align = 0,
                   Font* font = 0, uint32 color = 0xFFFFFFFF);
    void PrintTextWithBorder(RenderContext* context, const char* text, const Recti& rect,
                             uint32 align, Font* font, uint32 Color, uint32 BorderColor);
};

} // namespace Renderer
} // namespace NFE
