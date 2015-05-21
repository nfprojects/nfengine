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

#define MAX_QUEUED_QUADS (4096)

class GuiRenderer : public RendererModule<GuiRenderer>
{
    // FT_Library mFreeTypeLibrary;

    void FlushQueue(RenderContext *pContext);
    void PushQuad(RenderContext *pContext, const GuiQuad& quad);
    void PushQuads(RenderContext *pContext, const GuiQuad* pQuads, UINT size);

public:
    Font* MakeFont(const char* pPath, int height);

    void SetTarget(RenderContext *pContext, IRenderTarget* pTarget);
    void DrawQuad(RenderContext *pContext, const Recti& rect, ITexture* pTexture, Rectf* pTexCoords,
                  UINT color, bool alpha = false);
    void PrintText(RenderContext *pContext, const char* pText, const Recti& Rect, uint32 Align = 0,
                   Font* pFont = 0, uint32 Color = 0xFFFFFFFF);
    void PrintTextWithBorder(RenderContext *pContext, const char* pText, const Recti& Rect, uint32 Align ,
                             Font* pFont, uint32 Color, uint32 BorderColor);
};

} // namespace Renderer
} // namespace NFE
