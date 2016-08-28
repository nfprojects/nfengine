/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level GUI Renderer
 */

#pragma once

#include "PCH.hpp"
#include "Font.hpp"
#include "Engine.hpp"

#include "nfCommon/Logger/Logger.hpp"

#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

#include "GuiRenderer.hpp"

namespace NFE {
namespace Renderer {

FT_Library gFreeTypeLibrary = nullptr;

bool Font::InitFreeType()
{
    if (FT_Init_FreeType(&gFreeTypeLibrary))
    {
        NFE_LOG_ERROR("Failed to initialize FreeType library. Font's won't be supported.");
        gFreeTypeLibrary = nullptr;
        return false;
    }

    return true;
}

void Font::ReleaseFreeType()
{
    if (gFreeTypeLibrary)
        FT_Done_FreeType(gFreeTypeLibrary);
}

Font::Font()
    : mSize(0)
{}

bool Font::GetTextSize(const char* text, int& width, int& lines, size_t numChars) const
{
    int maxOffsetX = 0, offsetX = 0;
    lines = 1;
    for (size_t i = 0; i < numChars && text[i]; ++i)
    {
        // TODO: UTF-8 support
        size_t charId = static_cast<unsigned char>(text[i]);
        if (charId > 127)
            charId = '?';

        if (text[i] == '\n')  // carriage return
        {
            lines++;
            maxOffsetX = std::max(maxOffsetX, offsetX);
            offsetX = 0;
            continue;
        }

        if (mChars[charId].width < 0)
            charId = '?';

        offsetX += mChars[charId].spacing;
    }

    width = offsetX;
    return true;
}

bool Font::Init(const char* file, int size)
{
    FT_Face face;
    int texWidth = 16 * size;
    int texHeight = 16 * size;

    // create pixels array
    std::unique_ptr<unsigned char[]> texData(new unsigned char[texWidth * texHeight]);

    if (FT_New_Face(gFreeTypeLibrary, file, 0, &face) != 0)
    {
        NFE_LOG_ERROR("Failed to load font '%s' of size %i.", file, size);
        return false;
    }

    if (FT_Set_Char_Size(face, 0, size * 64, 96, 96))
    {
        NFE_LOG_ERROR("FT_Set_Char_Size() failed");
        return false;
    }

    /// glyph offset in the texture
    int offsetX = 0;
    int offsetY = 0;

    const size_t charsNum = 128;
    mSize = size;
    mChars.resize(charsNum);

    for (size_t charId = 0; charId < charsNum; ++charId)
    {
        // load the glyph for our character
        unsigned int glyphIndex = FT_Get_Char_Index(face, static_cast<FT_ULong>(charId));
        if (glyphIndex == 0)
            continue;

        if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))
            continue;

        // move the face's glyph into a glyph object
        FT_Glyph glyph;
        FT_Get_Glyph(face->glyph, &glyph);

        // convert the glyph to a bitmap
        FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
        FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

        // this reference will make accessing the bitmap easier
        FT_Bitmap& bitmap = bitmap_glyph->bitmap;

        // char won't fit to this row
        if (offsetX + bitmap.width + 2 > texWidth)
        {
            offsetX = 0;
            offsetY += 2 * size;

            if (offsetY >= texHeight)
            {
                NFE_LOG_WARNING("Texture too small");
                break;
            }
        }

        for (int y = 0; y < bitmap.rows; y++)
        {
            for (int x = 0; x < bitmap.width; x++)
            {
                unsigned char value = bitmap.buffer[x + bitmap.width * y];
                texData[(y + offsetY) * texWidth + x + offsetX] = value;
            }
        }

        mChars[charId].top = static_cast<short>(bitmap_glyph->top);
        mChars[charId].left = static_cast<short>(bitmap_glyph->left);
        mChars[charId].height = static_cast<short>(bitmap.rows);
        mChars[charId].width = static_cast<short>(bitmap.width);
        mChars[charId].u = static_cast<short>(offsetX);
        mChars[charId].v = static_cast<short>(offsetY);
        mChars[charId].spacing = static_cast<short>(face->glyph->advance.x >> 6);

        offsetX += bitmap.width + 1;
        FT_Done_Glyph(glyph);
    }

    // close font
    FT_Done_Face(face);

    mTexHeight = offsetY + 2 * size;
    mTexWidth = texWidth;

    std::string debugName;
    debugName = std::string("Font::mTexture name=") + file + ", size=" + std::to_string(size);

    TextureDataDesc texDataDesc;
    texDataDesc.lineSize = texDataDesc.sliceSize = mTexWidth * sizeof(uint8);
    texDataDesc.data = texData.get();

    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.mode = BufferMode::Static;
    texDesc.width = mTexWidth;
    texDesc.height = mTexHeight;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    texDesc.mipmaps = 1;
    texDesc.dataDesc = &texDataDesc;
    texDesc.format = ElementFormat::R8_U_Norm;
    texDesc.debugName = debugName.c_str();

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    mTexture = renderer->GetDevice()->CreateTexture(texDesc);
    if (!mTexture)
        return false;

    mTextureBinding = GuiRenderer::Get()->CreateTextureBinding(mTexture);
    if (!mTextureBinding)
        return false;

    NFE_LOG_SUCCESS("Font '%s' of size %i loaded successfully", file, size);
    return true;
}


} // namespace Renderer
} // namespace NFE
