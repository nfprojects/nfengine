/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   OGL4 implementation of renderer's backbuffer
 */

#include "PCH.hpp"

#include "../Defines.hpp"
#include "../Backbuffer.hpp"
#include "../Extensions.hpp"
#include "../MasterContext.hpp"
#include "ContextDataGLX.hpp"


namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : mWindow(0)
    , mMasterDisplay(nullptr)
    , mContext(0)
    , mDrawable(0)
    , mDummyVAO(0)
{
}

Backbuffer::~Backbuffer()
{
    glBindVertexArray(GL_NONE);
    glDeleteVertexArrays(1, &mDummyVAO);
    glXMakeCurrent(mMasterDisplay, None, 0);
    glXDestroyContext(mMasterDisplay, mContext);
    mMasterDisplay = nullptr;
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

    const void* dataRaw = MasterContext::Instance().GetData();
    if (dataRaw == nullptr)
    {
        LOG_ERROR("Cannot initialize Backbuffer - Master Context is not initialized");
        return false;
    }
    const ContextDataGLX* data = reinterpret_cast<const ContextDataGLX*>(dataRaw);
    mMasterDisplay = data->mDisplay;

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

        mContext = glXCreateContextAttribsARB(mMasterDisplay, data->mBestFB,
                                              data->mContext, GL_TRUE, attribs);
        if (!mContext)
        {
            LOG_WARNING("GL 3.3 or newer not acquired. Falling back to old one.");
            LOG_WARNING("Keep in mind, the renderer MIGHT NOT WORK due to too old OGL version!");
            // failed, fallback to classic method
            mContext = glXCreateNewContext(mMasterDisplay, data->mBestFB,
                                           GLX_RGBA_TYPE, data->mContext, GL_TRUE);
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
        mContext = glXCreateNewContext(mMasterDisplay, data->mBestFB,
                                       GLX_RGBA_TYPE, data->mContext, GL_TRUE);
    }
    glXMakeCurrent(mMasterDisplay, mWindow, mContext);
    mDrawable = glXGetCurrentDrawable();

    if (!glXIsDirect(mMasterDisplay, mContext))
        LOG_INFO("Indirect GLX Slave Context obtained");
    else
        LOG_INFO("Direct GLX Slave Context obtained");

    // some systems might have glXSwapIntervalEXT unavailable
    if (glXSwapIntervalEXT)
        glXSwapIntervalEXT(mMasterDisplay, mDrawable, desc.vSync);
    else
        LOG_WARNING("glXSwapIntervalEXT was not acquired, VSync control is disabled.");

    // On Linux using OGL Core Profile a VAO must be bound to the pipeline for rendering. It can
    // be any VAO, however without it no drawing is performed.
    // Since VAOs are not shared between contexts, it is best to assume that each context
    // will create its own VAO for use.
    glGenVertexArrays(1, &mDummyVAO);
    glBindVertexArray(mDummyVAO);

    return true;
}

bool Backbuffer::Present()
{
    // Function behavior is the same as its Win version.
    // See Win/BackBuffer.cpp for more details.
    unsigned int vSync;
    glXQueryDrawable(mMasterDisplay, mDrawable, GLX_SWAP_INTERVAL_EXT, &vSync);
    if (!vSync)
       glFinish();

    glXSwapBuffers(mMasterDisplay, mDrawable);
    return true;
}

} // namespace Renderer
} // namespace NFE
