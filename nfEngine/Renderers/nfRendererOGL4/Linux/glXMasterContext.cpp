/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OpenGL Master Context singleton definitions for Linux
 */

#include "../MasterContext.hpp"

#include "glXContextData.hpp"


namespace NFE {
namespace Renderer {


MasterContext::MasterContext()
    : mData(nullptr)
    , mPixmap(None)
    , mGLXPixmap(None)
    , mDrawable(None)
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
    glXContextData data;

    // connect to a display
    data.mDisplay = ::XOpenDisplay(0);

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
    GLXFBConfig* fbc = glXChooseFBConfig(data.mDisplay, DefaultScreen(data.mDisplay),
                                         fbAttribs, &fbCount);

    // Select the best FB Config according to highest SAMPLES count
    // FIXME temporarily it is the easiest method for us to get the same FB as in Window.cpp
    //       In the future, this must be acquired from Window.
    int bestFBID = -1, maxSamples = 16;
    for (int i = 0; i < fbCount; ++i)
    {
        XVisualInfo* vi = glXGetVisualFromFBConfig(data.mDisplay, fbc[i]);
        if (vi)
        {
            int samples;
            glXGetFBConfigAttrib(data.mDisplay, fbc[i], GLX_SAMPLES, &samples);

            if (samples < maxSamples)
            {
                bestFBID = i;
                maxSamples = samples;
            }
        }
        XFree(vi);
    }

    LOG_INFO("Master Context selected FB Config #%d", bestFBID);
    data.mBestFB = fbc[bestFBID];
    XFree(fbc);

    // create a pixmap for our context
    // this is required for off-screen context to work
    // TODO check if bit-depth (24) should match currently set bit depth in X
    mPixmap = XCreatePixmap(data.mDisplay, DefaultRootWindow(data.mDisplay), 1, 1, 24);
    mGLXPixmap = glXCreatePixmap(data.mDisplay, data.mBestFB, mPixmap, NULL);

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

        data.mContext = glXCreateContextAttribsARB(data.mDisplay, data.mBestFB,
                                                   NULL, GL_TRUE, attribs);
        if (!data.mContext)
        {
            LOG_WARNING("GL 3.3 or newer not acquired. Falling back to old one.");
            LOG_WARNING("Keep in mind, the renderer MIGHT NOT WORK due to too old OGL version!");
            // failed, fallback to classic method
            data.mContext = glXCreateNewContext(data.mDisplay, data.mBestFB,
                                                GLX_RGBA_TYPE, NULL, GL_TRUE);
            if (!data.mContext)
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
        data.mContext = glXCreateNewContext(data.mDisplay, data.mBestFB,
                                            GLX_RGBA_TYPE, NULL, GL_TRUE);
        if (!data.mContext)
        {
            LOG_ERROR("Cannot create OpenGL Master Context.");
            return false;
        }
    }
    glXMakeCurrent(data.mDisplay, mGLXPixmap, data.mContext);
    mDrawable = glXGetCurrentDrawable();

    if (!glXIsDirect(data.mDisplay, data.mContext))
        LOG_INFO("Indirect GLX Master Context obtained");
    else
        LOG_INFO("Direct GLX Master Context obtained");

    mData = reinterpret_cast<void*>(new glXContextData(data));

    return true;
}

void MasterContext::Release()
{
    if (mData)
    {
        glXContextData* data = reinterpret_cast<glXContextData*>(mData);

        if (data->mDisplay)
        {
            glXMakeCurrent(data->mDisplay, None, 0);

            glXDestroyContext(data->mDisplay, data->mContext);
            data->mContext = None;

            glXDestroyPixmap(data->mDisplay, mGLXPixmap);
            XFreePixmap(data->mDisplay, mPixmap);
            mPixmap = None;

            XCloseDisplay(data->mDisplay);
            data->mDisplay = nullptr;
        }

        // free unused data
        delete data;
        mData = nullptr;
    }
}

const void* MasterContext::GetData()
{
    return mData;
}

} // namespace Renderer
} // namespace NFE
