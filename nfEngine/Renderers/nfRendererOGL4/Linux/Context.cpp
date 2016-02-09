/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OpenGL Main Context singleton declaration
 */

#include "../Context.hpp"

#include "ContextDataGLX.hpp"
#include "GL/glx.h"

namespace NFE {
namespace Renderer {


Context::Context()
    : mDisplay(nullptr)
    , mPixmap(None)
    , mGLXPixmap(None)
    , mContext(None)
    , mDrawable(None)
{
}

Context::~Context()
{
    glXMakeCurrent(mDisplay, None, 0);
    glXDestroyContext(mDisplay, mContext);
    glXDestroyPixmap(mDisplay, mGLXPixmap);
    XFreePixmap(mDisplay, mPixmap);
    XCloseDisplay(mDisplay);
}

static Context& Context::Instance()
{
    static Context instance;
    return instance;
}

bool Context::Init()
{
    ContextDataGLX data;

    // connect to a display
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
        XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc[i]);
        if (vi)
        {
            int samples;
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES, &samples);

            if (samples < maxSamples)
            {
                bestFBID = i;
                maxSamples = samples;
            }
        }
        XFree(vi);
    }

    LOG_INFO("Context selected FB Config #%d", bestFBID);
    data.mBestFB = fbc[bestFBID];
    XFree(fbc);

    // create a pixmap for our context
    // this is required for off-screen context to work
    // TODO check if bit-depth (24) should match currently set bit depth in X
    mPixmap = XCreatePixmap(mDisplay, DefaultRootWindow(mDisplay), 1, 1, 24);
    mGLXPixmap = glXCreatePixmap(mDisplay, data.mBestFB, mPixmap, NULL);

    // create OGL context
    if (glXCreateContextAttribsARB)
    {
        // try reaching for 3.3 core context
        int attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            None
        };

        mContext = glXCreateContextAttribsARB(mDisplay, data.mBestFB, NULL, GL_TRUE, attribs);
        if (!mContext)
        {
            LOG_WARNING("GL 3.3 or newer not acquired. Falling back to old one.");
            LOG_WARNING("Keep in mind, the renderer MIGHT NOT WORK due to too old OGL version!");
            // failed, fallback to classic method
            mContext = glXCreateNewContext(mDisplay, data.mBestFB, GLX_RGBA_TYPE, NULL, GL_TRUE);
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
        mContext = glXCreatetaNewContext(mDisplay, data.mBestFB, GLX_RGBA_TYPE, NULL, GL_TRUE);
    }
    glXMakeCurrent(mDisplay, mGLXPixmap, mContext);
    mDrawable = glXGetCurrentDrawable();

    if (!glXIsDirect(mDisplay, mContext))
        LOG_INFO("Indirect GLX rendering context obtained");
    else
        LOG_INFO("Direct GLX rendering context obtained");

    mData = reinterpret_cast<void*>(&data);

    return true;
}


} // namespace Renderer
} // namespace NFE
