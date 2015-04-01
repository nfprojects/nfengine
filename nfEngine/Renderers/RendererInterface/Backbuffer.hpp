/**
 * @file    Backbuffer.hpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Texture.hpp"
#include "Types.hpp"

namespace NFE {
namespace Renderer {

/**
 * Description of a backbuffer (a texture that represents window surface).
 */
struct BackbufferDesc
{
    int width;
    int height;
    void* windowHandle; //< platform dependent window handle

    // TODO: options such as: UAV usage, mulitsampling, texture format, etc.
};

class IBackbuffer : virtual public ITexture
{
public:
    virtual ~IBackbuffer() {}

    // should be called when window has been resized
    virtual bool Resize(int newWidth, int newHeight) = 0;

    // swap buffers
    virtual bool Present() = 0;
};

} // namespace Renderer
} // namespace NFE
