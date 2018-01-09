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

    NFE_LOG_INFO("Master Context selected FB Config #%d", bestFBID);
    mBestFB = fbc[bestFBID];
    XFree(fbc);

    // create a pixmap for our context
    // this is required for off-screen context to work
    // TODO check if bit-depth (24) should match currently set bit depth in X
    mPixmap = XCreatePixmap(mDisplay, DefaultRootWindow(mDisplay), 1, 1, 24);
    mGLXPixmap = glXCreatePixmap(mDisplay, mBestFB, mPixmap, NULL);

    if (!glXCreateContextAttribsARB)
    {
        NFE_LOG_ERROR("Failed to extract glXCreateContextAttribsARB.");
        return false;
    }

    // TODO debug context
    int attribs[] =
    {
        // use Core Profile
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        // disable deprecated APIs
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        // select context version
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        None
    };

    // create our requested context
    mContext = glXCreateContextAttribsARB(mDisplay, mBestFB, NULL, GL_TRUE, attribs);
    if (!mContext)
    {
        NFE_LOG_ERROR("Failed to create OpenGL Master Context.");
        return false;
    }

    glXMakeCurrent(mDisplay, mGLXPixmap, mContext);
    mDrawable = glXGetCurrentDrawable();

    // For information purposes - print what OpenGL Context we achieved
    const GLubyte* glv = glGetString(GL_VERSION);
    const char* glvStr = reinterpret_cast<const char*>(glv);
    bool direct = glXIsDirect(mDisplay, mContext);
    NFE_LOG_INFO("OpenGL %s %s Master Context obtained.", glvStr,
             direct ? "Direct" : "Indirect");

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
