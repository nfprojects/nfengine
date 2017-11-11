/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level GUI Renderer
 */

#pragma once

#include "HighLevelRenderer.hpp"

namespace NFE {
namespace Renderer {

struct CharacterInfo
{
    int16 width, height;
    int16 spacing;
    int16 top, left, u, v;

    CharacterInfo()
        : width(-1)
        , height(-1)
        , spacing(0)
        , top(0)
        , left(0)
        , u(0)
        , v(0)
    {}
};

/**
 * Bitmap-based font resource.
 */
class NFE_RENDERER_API Font
{
    friend class GuiRenderer;

    TexturePtr mTexture;
    ResourceBindingInstancePtr mTextureBinding; // GUI Renderer texture binding
    std::vector<CharacterInfo> mChars;
    int mSize;
    int mTexWidth, mTexHeight;

public:
    Font();

    /**
     * Create a font.
     * @param file Font file path.
     * @param size Font size in pixels.
     * @return True on success.
     */
    bool Init(const char* file, int size);

    /**
     * Calculate text width and lines number.
     */
    bool GetTextSize(const char* text, int& width, int& lines) const;

    static bool InitFreeType();

    static void ReleaseFreeType();
};


} // namespace Renderer
} // namespace NFE
