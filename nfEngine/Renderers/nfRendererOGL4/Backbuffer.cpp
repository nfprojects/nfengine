/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   OGL4 implementation of renderer's backbuffer
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Backbuffer.hpp"


namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : mHWND(nullptr)
    , mHDC(nullptr)
    , mHRC(nullptr)
{
}

Backbuffer::~Backbuffer()
{
    if (mHRC)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(mHRC);
        mHRC = nullptr;
    }

    if (mHDC)
    {
        ReleaseDC(mHWND, mHDC);
        mHDC = nullptr;
    }
}

bool Backbuffer::Resize(int newWidth, int newHeight)
{
    UNUSED(newWidth);
    UNUSED(newHeight);

    return 0;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    type = TextureType::Texture2D;
    mHWND = static_cast<HWND>(desc.windowHandle);
    mWidth = desc.width;
    mHeight = desc.height;

    // Initialize OpenGL
    // FIXME OGL initialization should be probably moved to Device. Unfortunately, current Device
    //       implementation is not OGL-friendly. To initialize OGL inside Device we need HWND,
    //       plus Backbuffer would need to have hDC passed for Backbuffer::Present. Needs research.

    // TODO PFD configuration
    PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
                                  PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                                  PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16,
                                  0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 };

    mHDC = GetDC(mHWND);
    if (!mHDC)
        return false;

    unsigned int pixelFormat = ChoosePixelFormat(mHDC, &pfd);
    SetPixelFormat(mHDC, pixelFormat, &pfd);

    mHRC = wglCreateContext(mHDC);
    if (!mHRC)
        return false;

    wglMakeCurrent(mHDC, mHRC);

    // OpenGL Extensions can be initialized only after GL context is current
    if (!nfglExtensionsInit())
        return false;

    // enable/disable vsync
    wglSwapIntervalEXT(desc.vSync);

    return true;
}

bool Backbuffer::Present()
{
    // OGL wiki recommends following scheme of work:
    //   * VSync enabled - just swap buffers, we can buy us some CPU work that way
    //   * VSync disabled - call glFinish before swap to make sure all calls are done

    // VSync is enabled if wglGetSwapIntervalEXT returns 1 or higher
    if (!wglGetSwapIntervalEXT())
        glFinish();

    if (SwapBuffers(mHDC))
        return false;
    else
        return true;
}

} // namespace Renderer
} // namespace NFE
