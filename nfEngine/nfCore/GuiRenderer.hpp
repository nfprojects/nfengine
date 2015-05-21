/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level GUI Renderer
 */

#pragma once

#include "Renderer.hpp"
#include "Multishader.hpp"

namespace NFE {
namespace Renderer {

// text alignment
#define X_TEXT_ALIGN_X_LEFT (1 << 0)
#define X_TEXT_ALIGN_X_CENTER (0)
#define X_TEXT_ALIGN_X_RIGHT (1 << 1)
#define X_TEXT_ALIGN_Y_TOP (1 << 2)
#define X_TEXT_ALIGN_Y_CENTER (0)
#define X_TEXT_ALIGN_Y_BOTTOM (1 << 3)

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

// TODO: remove inheritance
class GuiRenderer
{
    FT_Library mFreeTypeLibrary;
    Font* mDefaultFont;

    // TODO: these must be per-context
    GuiQuad* mQuads;
    UINT mQueuedQuads;

    void FlushQueue(NFE_CONTEXT_ARG);
    void PushQuad(NFE_CONTEXT_ARG, const GuiQuad& quad);
    void PushQuads(NFE_CONTEXT_ARG, const GuiQuad* pQuads, UINT size);

public:
    GuiRenderer();
    ~GuiRenderer();

    // TODO: move to the engine's core
    Font* MakeFont(const char* pPath, int height);

    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    void SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget);
    void DrawQuad(NFE_CONTEXT_ARG, const Recti& rect, ITexture* pTexture, Rectf* pTexCoords,
                  UINT color, bool alpha = false);
    void PrintText(NFE_CONTEXT_ARG, const char* pText, const Recti& Rect, uint32 Align = 0,
                   Font* pFont = 0, uint32 Color = 0xFFFFFFFF);
    void PrintTextWithBorder(NFE_CONTEXT_ARG, const char* pText, const Recti& Rect, uint32 Align ,
                             Font* pFont, uint32 Color, uint32 BorderColor);
};

} // namespace Renderer
} // namespace NFE
