/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   OGL4 implementation of renderer's backbuffer
 */

#include "PCH.hpp"

#include "../Defines.hpp"
#include "../Backbuffer.hpp"

#include "../Extensions.hpp"


namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : mWindow(0)
    , mDisplay(nullptr)
    , mContext(0)
    , mDrawable(0)
{
}

Backbuffer::~Backbuffer()
{
    glXMakeCurrent(0, 0, 0);
    glXDestroyContext(mDisplay, mContext);
    XCloseDisplay(mDisplay);
    mWindow = 0;
}

bool Backbuffer::Resize(int newWidth, int newHeight)
{
    UNUSED(newWidth);
    UNUSED(newHeight);

    return 0;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    // get window and connect to a display
    mWindow = reinterpret_cast<Window>(desc.windowHandle);
    mDisplay = ::XOpenDisplay(0);

    // acquire window's attributes
    XWindowAttributes winAttrs;
    XGetWindowAttributes(mDisplay, mWindow, &winAttrs);

    // acquire matching XVisualInfo using window's visual pointer
    XVisualInfo* visualInfo;
    XVisualInfo visualInfoTemplate;
    visualInfoTemplate.visual = winAttrs.visual;
    visualInfoTemplate.screen = 0;
    int visualsCount = 0;
    visualInfo = XGetVisualInfo(mDisplay, VisualScreenMask, &visualInfoTemplate, &visualsCount);
    if (visualsCount == 0)
    {
        // no visuals found, error
        LOG_ERROR("Unable to retrieve matching Visual from window");
        return false;
    }
    else if (visualsCount > 1)
    {
        LOG_INFO("%i visuals found", visualsCount);
    }

    // create OGL context
    mContext = glXCreateContext(mDisplay, visualInfo, NULL, GL_TRUE);
    glXMakeCurrent(mDisplay, mWindow, mContext);
    mDrawable = glXGetCurrentDrawable();

    // acquire OGL extensions
    if (!nfglExtensionsInit())
        return false;

    // some systems might have glXSwapIntervalEXT unavailable
    if (glXSwapIntervalEXT)
        glXSwapIntervalEXT(mDisplay, mDrawable, desc.vSync);
    else
        LOG_WARNING("glXSwapIntervalEXT was not acquired, VSync control is disabled.");

    return true;
}

bool Backbuffer::Present()
{
    // Function behavior is th same as its Win version.
    // See Win/BackBuffer.cpp for more details.
    unsigned int vSync;
    glXQueryDrawable(mDisplay, mDrawable, GLX_SWAP_INTERVAL_EXT, &vSync);
    if (!vSync)
       glFinish();

    glXSwapBuffers(mDisplay, mDrawable);
    return true;
}

} // namespace Renderer
} // namespace NFE
