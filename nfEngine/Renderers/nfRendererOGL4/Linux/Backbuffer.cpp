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
    glXMakeCurrent(mDisplay, None, 0);
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

    // acquire OGL extensions
    if (!nfglExtensionsInit())
        return false;

    static int fbAttribs[] =
    {
        GLX_X_RENDERABLE,  True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE,      8,
        GLX_GREEN_SIZE,    8,
        GLX_BLUE_SIZE,     8,
        GLX_ALPHA_SIZE,    8,
        GLX_DEPTH_SIZE,    24,
        GLX_STENCIL_SIZE,  8,
        GLX_DOUBLEBUFFER,  True,
        None
    };

    int fbCount;
    GLXFBConfig* fbc = glXChooseFBConfig(mDisplay, DefaultScreen(mDisplay), fbAttribs, &fbCount);

    // Select the best FB Config according to highest SAMPLES count
    // FIXME temporarily it is the easiest method for us to get the same FB as in Window.cpp
    //       In the future, this must be acquired from Window.
    int bestFBID = -1, maxSamples = 16;
    for (int i = 0; i < fbCount; ++i)
    {
        XVisualInfo* vi = glXGetVisualFromFBConfig(mDisplay, fbc[i]);
        if (vi)
        {
            int samples;
            glXGetFBConfigAttrib(mDisplay, fbc[i], GLX_SAMPLES, &samples);

            if (samples < maxSamples)
            {
                bestFBID = i;
                maxSamples = samples;
            }
        }
        XFree(vi);
    }

    LOG_INFO("Renderer selected FB Config #%d", bestFBID);
    GLXFBConfig bestFB = fbc[bestFBID];
    XFree(fbc);

    // create OGL context
    if (glXCreateContextAttribsARB)
    {
        // try reaching for 3.0 core context
        int attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            None
        };

        mContext = glXCreateContextAttribsARB(mDisplay, bestFB, NULL, GL_TRUE, attribs);
        if (!mContext)
        {
            LOG_WARNING("GL 3.3 or newer not acquired. Falling back to old one.");
            LOG_WARNING("Keep in mind, the renderer MIGHT NOT WORK due to too old OGL version!");
            // failed, fallback to classic method
            mContext = glXCreateNewContext(mDisplay, bestFB, GLX_RGBA_TYPE, NULL, GL_TRUE);
            if (!mContext)
            {
                LOG_ERROR("Cannot create OpenGL Context.");
                return false;
            }
        }
    }
    else
    {
        LOG_WARNING("glXCreateContextAttribsARB not available. Creating OGL context the old way.");
        LOG_WARNING("Keep in mind, the renderer MIGHT NOT WORK due to too old OGL version!");
        mContext = glXCreateNewContext(mDisplay, bestFB, GLX_RGBA_TYPE, NULL, GL_TRUE);
    }
    glXMakeCurrent(mDisplay, mWindow, mContext);
    mDrawable = glXGetCurrentDrawable();

    if (!glXIsDirect(mDisplay, mContext))
        LOG_INFO("Indirect GLX rendering context obtained");
    else
        LOG_INFO("Direct GLX rendering context obtained");

    // some systems might have glXSwapIntervalEXT unavailable
    if (glXSwapIntervalEXT)
        glXSwapIntervalEXT(mDisplay, mDrawable, desc.vSync);
    else
        LOG_WARNING("glXSwapIntervalEXT was not acquired, VSync control is disabled.");

    return true;
}

bool Backbuffer::Present()
{
    // Function behavior is the same as its Win version.
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
