/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OpenGL Master Context singleton definitions for Linux
 */

#include "../MasterContext.hpp"


namespace NFE {
namespace Renderer {


MasterContext::MasterContext()
    : mPixmap(None)
    , mGLXPixmap(None)
    , mDrawable(None)
    , mDisplay(nullptr)
    , mBestFB()
    , mContext(None)
{
}

MasterContext::~MasterContext()
{
    Release();
}

MasterContext& MasterContext::Instance()
{
    static MasterContext instance;
    return instance;
}

bool MasterContext::Init()
{
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

    LOG_INFO("Master Context selected FB Config #%d", bestFBID);
    mBestFB = fbc[bestFBID];
    XFree(fbc);

    // create a pixmap for our context
    // this is required for off-screen context to work
    // TODO check if bit-depth (24) should match currently set bit depth in X
    mPixmap = XCreatePixmap(mDisplay, DefaultRootWindow(mDisplay), 1, 1, 24);
    mGLXPixmap = glXCreatePixmap(mDisplay, mBestFB, mPixmap, NULL);

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

        mContext = glXCreateContextAttribsARB(mDisplay, mBestFB, NULL, GL_TRUE, attribs);
        if (!mContext)
        {
            LOG_WARNING("GL 3.3 or newer not acquired. Falling back to old one.");
            LOG_WARNING("Keep in mind, the renderer MIGHT NOT WORK due to too old OGL version!");
            // failed, fallback to classic method
            mContext = glXCreateNewContext(mDisplay, mBestFB, GLX_RGBA_TYPE, NULL, GL_TRUE);
            if (!mContext)
            {
                LOG_ERROR("Cannot create OpenGL Master Context.");
                return false;
            }
        }
    }
    else
    {
        LOG_WARNING("glXCreateContextAttribsARB not available. Creating OGL context the old way.");
        LOG_WARNING("Keep in mind, the renderer MIGHT NOT WORK due to too old OGL version!");
        mContext = glXCreateNewContext(mDisplay, mBestFB, GLX_RGBA_TYPE, NULL, GL_TRUE);
        if (!mContext)
        {
            LOG_ERROR("Cannot create OpenGL Master Context.");
            return false;
        }
    }
    glXMakeCurrent(mDisplay, mGLXPixmap, mContext);
    mDrawable = glXGetCurrentDrawable();

    if (!glXIsDirect(mDisplay, mContext))
        LOG_INFO("Indirect GLX Master Context obtained");
    else
        LOG_INFO("Direct GLX Master Context obtained");

    return true;
}

void MasterContext::Release()
{
    if (mDisplay)
    {
        glXMakeCurrent(mDisplay, None, 0);

        glXDestroyContext(mDisplay, mContext);
        mContext = None;

        glXDestroyPixmap(mDisplay, mGLXPixmap);
        XFreePixmap(mDisplay, mPixmap);
        mPixmap = None;

        XCloseDisplay(mDisplay);
        mDisplay = nullptr;
    }
}

} // namespace Renderer
} // namespace NFE
