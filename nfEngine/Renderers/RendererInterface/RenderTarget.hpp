/**
 * @file    RenderTarget.hpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Texture.hpp"
#include "Types.hpp"

namespace NFE {
namespace Renderer {

/**
 * Render target description.
 */
struct RenderTargetDesc
{
    int width;
    int height;
    void* windowHandle;
};

class IRenderTarget : virtual public ITexture
{
public:
    virtual ~IRenderTarget() {}

    // should be called when window has been resized
    virtual int Resize(int newWidth, int newHeight) = 0;

    // swap buffers
    virtual int Present() = 0;
};

} // namespace Renderer
} // namespace NFE
