#include "PCH.hpp"
#include "GuiRenderer.hpp"
#include "Renderer.hpp"
#include "../nfCommon/Image.hpp"
#include "../nfCommon/Timer.hpp"

namespace NFE {
namespace Render {

using namespace Math;

int Font::GetTextWidth(const char* pText)
{
    int i = 0;
    int OffsetX = 0;
    while (pText[i])
    {
        int CharId = (int)(pText[i]);

        if (!characters[CharId].exists || CharId > 65536)
            CharId = 0x003F;

        OffsetX += characters[CharId].spacing;
        i++;
    }

    return OffsetX;
}

GuiRendererD3D11::GuiRendererD3D11()
{
    mDefaultFont = NULL;

    mFreeTypeLibrary = 0;

    mVertexBuffer = NULL;
    mInputLayout = NULL;
    mCBuffer = NULL;

    mRasterizerState = NULL;
    mBlendState = NULL;

    mQuads = NULL;
    mQueuedQuads = 0;
}

GuiRendererD3D11::~GuiRendererD3D11()
{
    Release();
}

Result GuiRendererD3D11::Init(RendererD3D11* pRenderer)
{
    FT_Error Err = FT_Init_FreeType(&mFreeTypeLibrary);
    if (Err)
    {
        //TODO
        //LOG_WARNING("Cannot initiazlize FreeType Library. Fonts won't be available.");
    }

    //compile shaders
    mShaderVS.Init(pRenderer, ShaderType::Vertex, "GuiVS");

    mShaderPS.AddMacro("USE_TEXTURE", 1, 0);
    mShaderPS.AddMacro("TEXTURE_MODE", 1, 1);
    mShaderPS.Init(pRenderer, ShaderType::Pixel, "GuiPS");

    mShaderGS.Init(pRenderer, ShaderType::Geometry, "GuiGS");


    D3D11_SUBRESOURCE_DATA initData;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    D3D11_BUFFER_DESC bd;
    bd.MiscFlags = 0;

    //init constant buffer with projection matrix
    bd.ByteWidth = sizeof(Matrix);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.StructureByteStride = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mCBuffer));

    //init Vertex Buffer
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = MAX_QUEUED_QUADS * sizeof(GuiVertex);
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.StructureByteStride = sizeof(GuiVertex);
    D3D_CHECK(pRenderer->D3DDevice->CreateBuffer(&bd, 0, &mVertexBuffer));


    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32A32_SINT,   0,  0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0,  16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",      0, DXGI_FORMAT_R8G8B8A8_UNORM,      0,  32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    //init Input Layout
    Common::Buffer* pShaderSource = mShaderVS.GetShaderBytecode(0);
    if (pShaderSource)
        D3D_CHECK(pRenderer->D3DDevice->CreateInputLayout(layout, 3, pShaderSource->GetData(),
                  pShaderSource->GetSize(), &mInputLayout));



    //allocate quads queue
    mQuads = (GuiQuad*)malloc(MAX_QUEUED_QUADS * sizeof(GuiQuad));
    mQueuedQuads = 0;


    // Initialize rasterizer state
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(rd));
    rd.CullMode = D3D11_CULL_NONE;
    rd.FrontCounterClockwise = 1;
    rd.FillMode = D3D11_FILL_SOLID;
    rd.DepthClipEnable = 1;
    D3D_CHECK(pRenderer->D3DDevice->CreateRasterizerState(&rd, &mRasterizerState));

    //alpha blendig
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = 0;
    blendDesc.IndependentBlendEnable = 0;
    blendDesc.RenderTarget[0].BlendEnable = 1;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    D3D_CHECK(pRenderer->D3DDevice->CreateBlendState(&blendDesc, &mBlendState));

    mDefaultFont = MakeFont("nfEngineTest\\Data\\Fonts\\Tahoma.ttf", 8);

    return Result::OK;
}

void GuiRendererD3D11::Release()
{
    mShaderVS.Release();
    mShaderGS.Release();
    mShaderPS.Release();

    D3D_SAFE_RELEASE(mVertexBuffer);
    D3D_SAFE_RELEASE(mInputLayout);
    D3D_SAFE_RELEASE(mCBuffer);

    D3D_SAFE_RELEASE(mRasterizerState);
    D3D_SAFE_RELEASE(mBlendState);

    //free quads queue
    if (mQuads)
    {
        free(mQuads);
        mQuads = 0;
    }

    //delete default font
    if (mDefaultFont)
    {
        delete mDefaultFont;
        mDefaultFont = 0;
    }

    //release free image library
    if (mFreeTypeLibrary)
    {
        FT_Done_FreeType(mFreeTypeLibrary);
        mFreeTypeLibrary = 0;
    }
}


unsigned int CeilToPowerOf2(unsigned int x)
{
    unsigned int n = 1;
    while (n < x) n <<= 1;
    return n;
}


Font::Font()
{
    texture = nullptr;
    characters = nullptr;
    height = 0;
    charCount = 0;
    texWidth = texHeight = 0;
}


Font::~Font()
{
    if (characters)
    {
        delete characters;
        characters = 0;
    }

    if (texture)
    {
        delete texture;
        texture = 0;
    }
}

/*
    TODO: Font rendering MUST be redesigned.
*/
Font* GuiRendererD3D11::MakeFont(const char* pPath, int height)
{
    Common::Timer timer;
    timer.Start();

    FT_Face face;
    int texWidth = 4096;
    int texHeight = 4096;

    //create pixels array
    unsigned char* pTexData = (unsigned char*)malloc(texWidth * texHeight);
    ZeroMemory(pTexData, texWidth * texHeight);

    if (FT_New_Face(mFreeTypeLibrary, pPath, 0, &face) != 0)
    {
        free(pTexData);
        // TODO
        // LOG_ERROR("Failed to load font '%s'.", pPath);
        return 0;
    }

    //FT_Set_Pixel_Sizes(face, 2*height, 2*height);
    FT_Set_Char_Size(face, 0, height * 64, 96, 96);


    Font* pFont = new Font;

    /*
    FT_Matrix Transform;
    Transform.xx = (FT_Fixed)(1.0f * 0x10000L);
    Transform.xy = (FT_Fixed)(0.0f * 0x10000L);
    Transform.yx = (FT_Fixed)(0.0f * 0x10000L);
    Transform.yy = (FT_Fixed)(1.0f * 0x10000L);
    FT_Set_Transform(face, &Transform, 0);
    */

    int Width;
    int Height;
    int OffsetX = 0;
    int OffsetY = 0;

    pFont->height = height;
    pFont->charCount = 65536;
    pFont->characters = (CharacterInfo*)malloc(sizeof(CharacterInfo) * pFont->charCount);

    int Index = 0;
    for (int ChrId = 0; ChrId < (65536); ChrId++)
    {
        // Load The Glyph For Our Character.
        unsigned int GlyphIndex = FT_Get_Char_Index(face, ChrId);
        if (GlyphIndex == 0)
        {
            pFont->characters[ChrId].exists = false;
            continue;
        }

        FT_Load_Glyph(face, GlyphIndex, FT_LOAD_DEFAULT);


        // Move The Face's Glyph Into A Glyph Object.
        FT_Glyph glyph;
        FT_Get_Glyph(face->glyph, &glyph);


        // Convert The Glyph To A Bitmap.
        FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
        FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

        // This Reference Will Make Accessing The Bitmap Easier.
        FT_Bitmap& bitmap = bitmap_glyph->bitmap;


        Width = bitmap.width;
        Height = bitmap.rows;

        //char won't fit to texture
        if (OffsetX + Width + 2 > texWidth)
        {
            OffsetX = 0;
            OffsetY += 2 * height;
        }

        for (int y = 0; y < Height; y++)
        {
            for (int x = 0; x < Width; x++)
            {
                unsigned char Value = bitmap.buffer[x + Width * y];

                int PxOffset = (y + OffsetY) * texWidth + x + OffsetX;
                pTexData[PxOffset] = Value;
            }
        }


        pFont->characters[ChrId].exists = true;
        pFont->characters[ChrId].top = (short)bitmap_glyph->top;
        pFont->characters[ChrId].left = (short)bitmap_glyph->left;
        pFont->characters[ChrId].height = (short)Height;
        pFont->characters[ChrId].width = (short)Width;
        pFont->characters[ChrId].u = (short)OffsetX;
        pFont->characters[ChrId].v = (short)OffsetY;
        pFont->characters[ChrId].spacing = (short)(face->glyph->advance.x >> 6) + 1;
        FT_Done_Glyph(glyph);

        OffsetX += pFont->characters[ChrId].width + 1;
        Index++;
    }

    //close font
    FT_Done_Face(face);

    OffsetY += 2 * height; // Used texture height
    texHeight = CeilToPowerOf2(OffsetY);


    Common::Image fontImage;
    fontImage.SetData(pTexData, texWidth, texHeight, Common::ImageFormat::A_UByte);
//  pFont->texture = pRenderer->CreateTexture(&fontImage, false);
    pFont->texture = new RendererTextureD3D11;
    pFont->texture->FromImage(fontImage);


    pFont->texHeight = texHeight;
    pFont->texWidth = texWidth;

    free(pTexData);

    // TODO
    // LOG_SUCCESS("Font '%s', size %i loaded in %.3lf seconds.", pPath, height, timer.Stop());
    return pFont;
}


void GuiRendererD3D11::Enter(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();

    //pass projection matrix to the vertex shader

    pCtx->D3DContext->GSSetConstantBuffers(0, 1, &mCBuffer);

    //init input assebler
    UINT stride = sizeof(GuiVertex);
    UINT offset = 0;
    pCtx->D3DContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    //pCtx->D3DContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    pCtx->D3DContext->IASetInputLayout(mInputLayout);
    pCtx->D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    pCtx->D3DContext->RSSetState(mRasterizerState);
    pCtx->D3DContext->OMSetBlendState(mBlendState, 0, 0xFFFFFFFF);

    pCtx->D3DContext->PSSetSamplers(0, 1, &pRenderer->defaultSampler);

}

void GuiRendererD3D11::Leave(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    auto pRenderer = pCtx->GetRenderer();
    FlushQueue(pCtx);

    pCtx->ResetShader(ShaderType::Geometry);
    pCtx->D3DContext->OMSetBlendState(pRenderer->defaultBlendState, 0, 0xFFFFFFFF);
}


void GuiRendererD3D11::SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget)
{
    auto pCtx = (RenderContextD3D11*)pContext;
    RenderTargetD3D11* pRT = dynamic_cast<RenderTargetD3D11*>(pTarget);

    pCtx->D3DContext->OMSetRenderTargets(1, &pRT->RTV, 0);
    Matrix projMatrix = MatrixOrtho(0.0f, (float)pRT->width, 0.0f, (float)pRT->height, -1.0f, 1.0f);
    pCtx->D3DContext->UpdateSubresource(mCBuffer, 0, 0, &projMatrix, 0, 0);
}

void GuiRendererD3D11::PushQuad(NFE_CONTEXT_ARG, const GuiQuad& quad)
{
    if (mQueuedQuads >= MAX_QUEUED_QUADS)
        FlushQueue(pContext);

    mQuads[mQueuedQuads] = quad;
    mQueuedQuads++;
}

void GuiRendererD3D11::PushQuads(NFE_CONTEXT_ARG, const GuiQuad* pQuads, UINT size)
{
    if (mQueuedQuads >= MAX_QUEUED_QUADS)
        FlushQueue(pContext);

    int i = 0;
    while (size)
    {
        mQuads[mQueuedQuads] = pQuads[i];
        i++;
        size--;

        if (mQueuedQuads >= MAX_QUEUED_QUADS)
            FlushQueue(pContext);
    }
}

void GuiRendererD3D11::FlushQueue(NFE_CONTEXT_ARG)
{
    auto pCtx = (RenderContextD3D11*)pContext;

    if (mQueuedQuads == 0)
        return;

    D3D11_MAPPED_SUBRESOURCE mappedVB;
    pCtx->D3DContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVB);
    GuiVertex* pVerticies = (GuiVertex*)mappedVB.pData;

    //build verticies array
    for (UINT i = 0; i < mQueuedQuads; i++)
    {
        pVerticies[i].rect = mQuads[i].rect;
        pVerticies[i].texCoord = mQuads[i].texCoord;
        pVerticies[i].color = mQuads[i].color;
    }

    //copy quads to the GPU
    pCtx->D3DContext->Unmap(mVertexBuffer, 0);

    UINT macros[2] = {0, 0}; // use texture, texture mode
    IRendererTexture* pTexture = NULL;
    bool alphaTexture = false;
    pCtx->BindShader(&mShaderVS, 0);
    pCtx->BindShader(&mShaderPS, 0);
    pCtx->BindShader(&mShaderGS, 0);

    UINT firstQuad = 0;
    UINT packetSize = 0;
    for (UINT i = 0; i < mQueuedQuads; i++)
    {
        if ((mQuads[i].pTexture != pTexture) || (mQuads[i].alphaTexture != alphaTexture))
        {
            //flush quads
            pCtx->D3DContext->Draw(packetSize, firstQuad);
            packetSize = 0;
            firstQuad = i;

            //change texture & shader
            pTexture = mQuads[i].pTexture;
            alphaTexture = mQuads[i].alphaTexture;
            macros[0] = (pTexture != NULL);
            macros[1] = (int)alphaTexture;
            pCtx->BindShader(&mShaderPS, macros);

            if (pTexture != NULL)
            {
                RendererTextureD3D11* pTextureD3D11 = dynamic_cast<RendererTextureD3D11*>(pTexture);
                pCtx->D3DContext->PSSetShaderResources(0, 1, &pTextureD3D11->SRV);
            }
        }

        packetSize++;
    }

    if (packetSize)
    {
        pCtx->D3DContext->Draw(packetSize, firstQuad);
    }

    mQueuedQuads = 0;
}

void GuiRendererD3D11::DrawQuad(NFE_CONTEXT_ARG, const Recti& rect, IRendererTexture* pTexture,
                                Rectf* pTexCoords, UINT color, bool alpha)
{
    GuiQuad quad;
    quad.rect = rect;
    quad.color = color;
    quad.pTexture = dynamic_cast<RendererTextureD3D11*>(pTexture);
    quad.alphaTexture = alpha;

    if ((pTexture != 0) && (pTexCoords != 0))
        quad.texCoord = *pTexCoords;
    else
    {
        quad.texCoord.Xmin = 0.0f;
        quad.texCoord.Ymin = 1.0f;
        quad.texCoord.Xmax = 1.0f;
        quad.texCoord.Ymax = 0.0f;
    }

    PushQuad(pContext, quad);
}



void GuiRendererD3D11::PrintText(NFE_CONTEXT_ARG, const char* pText, const Recti& Rect,
                                 uint32 Align , Font* pFont, uint32 Color)
{
    if (pFont == 0) pFont = mDefaultFont;
    if (pFont == 0) return;
    if (pFont->texture == 0) return;

    float texInvWidth = 1.0f / (float)pFont->texWidth;
    float texInvHeight = 1.0f / (float)pFont->texHeight;

    int OffsetX = Rect.Xmin;
    int OffsetY = Rect.Ymin;

    int TextLength = pFont->GetTextWidth(pText);
    int TextHeight = pFont->height;

    if (!(Align & X_TEXT_ALIGN_X_LEFT) && !(Align & X_TEXT_ALIGN_X_RIGHT))
        OffsetX = Rect.Xmin + (Rect.Xmax - Rect.Xmin - TextLength) / 2;
    if (!(Align & X_TEXT_ALIGN_X_LEFT) && (Align & X_TEXT_ALIGN_X_RIGHT))
        OffsetX = Rect.Xmin + (Rect.Xmax - Rect.Xmin - TextLength);

    if (!(Align & X_TEXT_ALIGN_Y_BOTTOM) && !(Align & X_TEXT_ALIGN_Y_TOP))
        OffsetY = Rect.Ymin + (Rect.Ymax - Rect.Ymin - TextHeight) / 2;
    if (!(Align & X_TEXT_ALIGN_Y_BOTTOM) && (Align & X_TEXT_ALIGN_Y_TOP))
        OffsetY = Rect.Ymin + (Rect.Ymax - Rect.Ymin - TextHeight);

    int DefaultOffsetX = OffsetX;

    GuiQuad quad;
    quad.pTexture = pFont->texture;
    quad.color = Color;
    quad.alphaTexture = true;

    int CharId;

    for (int i = 0; pText[i] != 0; i++)
    {
        CharId = (int)pText[i];

        if (CharId == 10 || CharId == 13)
        {
            OffsetY -= 2 * pFont->height;
            OffsetX = DefaultOffsetX;
            continue;
        }

        const CharacterInfo& charInfo = pFont->characters[CharId];

        if (CharId > 65536 || !charInfo.exists)
            CharId = 0x003F;


        quad.rect.Xmin = OffsetX + charInfo.left;
        quad.rect.Xmax = quad.rect.Xmin + charInfo.width;
        quad.rect.Ymin = OffsetY + charInfo.top - charInfo.height;
        quad.rect.Ymax = quad.rect.Ymin + charInfo.height;

        quad.texCoord.Xmin = (float)(charInfo.u) * texInvWidth;
        quad.texCoord.Ymax = (float)(charInfo.v) * texInvHeight;
        quad.texCoord.Xmax = (float)(charInfo.u + charInfo.width) * texInvWidth;
        quad.texCoord.Ymin = (float)(charInfo.v + charInfo.height) * texInvHeight;

        PushQuad(pContext, quad);
        OffsetX += charInfo.spacing;
    }
}

void GuiRendererD3D11::PrintTextWithBorder(NFE_CONTEXT_ARG, const char* pText, const Recti& Rect,
        uint32 Align , Font* pFont, uint32 Color, uint32 BorderColor)
{
    Recti tempRect;

    for (int i = -1; i < 2; i++)
    {
        for (int j = -1; j < 2; j++)
        {
            if (i == 0 && j == 0) continue;

            tempRect.Ymin = Rect.Ymin + j;
            tempRect.Ymax = Rect.Ymax + j;
            tempRect.Xmin = Rect.Xmin + i;
            tempRect.Xmax = Rect.Xmax + i;

            PrintText(pContext, pText, tempRect, Align, pFont, BorderColor);
        }
    }

    PrintText(pContext, pText, Rect, Align, pFont, Color);
}

} // namespace Render
} // namespace NFE
