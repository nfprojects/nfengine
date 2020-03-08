/**
 * @file
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
    void* windowHandle; //< platform dependent window handle
    int width;
    int height;
    ElementFormat format;
    bool vSync;
    const char* debugName; //< optional debug name
    // TODO: options such as: UAV usage, mulitsampling, texture format, etc.
    // TODO: buffers number (double buffering, triple buffering, etc.)

    BackbufferDesc()
        : windowHandle(nullptr)
        , width(0)
        , height(0)
        , format(ElementFormat::R8G8B8A8_U_Norm)
        , vSync(false)
        , debugName(nullptr)
    {
    }
};

class IBackbuffer
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
