#pragma once
#include "nfRendererD3D11.h"
#include "../nfCore/Renderer.h"
#include "Multishader.h"

namespace NFE {
namespace Render {

// text alignment
#define X_TEXT_ALIGN_X_LEFT (1 << 0)
#define X_TEXT_ALIGN_X_CENTER (0)
#define X_TEXT_ALIGN_X_RIGHT (1 << 1)
#define X_TEXT_ALIGN_Y_TOP (1 << 2)
#define X_TEXT_ALIGN_Y_CENTER (0)
#define X_TEXT_ALIGN_Y_BOTTOM (1 << 3)


class RendererTextureD3D11;

struct CharacterInfo
{
    bool exists;
    short width, height;
    short spacing;
    short top, left, u, v;
    unsigned int glyphIndex;
};

// TODO: move to the engine's core
class Font
{
public:
    IRendererTexture* texture;
    CharacterInfo* characters;
    int height;
    int charCount;

    int texWidth, texHeight;

    Font();
    ~Font();

    //bool Build(const char* pFileName);
    //void Write(const wchar_t* pText, int Count, DWORD Color, UINT Align, uiRect* pRect, uiRect* pCropRect);
    int GetTextWidth(const char* pText);
    //int GetLength(const wchar_t* pText, int Count);
};

struct GuiQuad
{
    Recti rect;
    Rectf texCoord;
    IRendererTexture* pTexture;
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


class GuiRendererD3D11 : public IGuiRenderer
{
    FT_Library mFreeTypeLibrary;
    Font* mDefaultFont;

    ID3D11Buffer* mVertexBuffer;
    ID3D11InputLayout* mInputLayout;
    ID3D11Buffer* mCBuffer;

    Multishader mShaderVS;
    Multishader mShaderPS;
    Multishader mShaderGS;

    ID3D11RasterizerState* mRasterizerState;
    ID3D11BlendState* mBlendState;

    // TODO: these must be per-context
    GuiQuad* mQuads;
    UINT mQueuedQuads;


    void FlushQueue(NFE_CONTEXT_ARG);
    void PushQuad(NFE_CONTEXT_ARG, const GuiQuad& quad);
    void PushQuads(NFE_CONTEXT_ARG, const GuiQuad* pQuads, UINT size);

public:
    GuiRendererD3D11();
    ~GuiRendererD3D11();

    Result Init(RendererD3D11* pRenderer);
    void Release();

    // TODO: move to the engine's core
    Font* MakeFont(const char* pPath, int height);

    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    void SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget);
    void DrawQuad(NFE_CONTEXT_ARG, const Recti& rect, IRendererTexture* pTexture, Rectf* pTexCoords,
                  UINT color, bool alpha = false);
    void PrintText(NFE_CONTEXT_ARG, const char* pText, const Recti& Rect, uint32 Align = 0,
                   Font* pFont = 0, uint32 Color = 0xFFFFFFFF);
    void PrintTextWithBorder(NFE_CONTEXT_ARG, const char* pText, const Recti& Rect, uint32 Align ,
                             Font* pFont, uint32 Color, uint32 BorderColor);
};

} // namespace Render
} // namespace NFE
