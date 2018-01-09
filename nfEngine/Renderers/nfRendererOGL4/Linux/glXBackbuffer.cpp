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


namespace NFE {
namespace Renderer {

Backbuffer::Backbuffer()
    : mWindow(None)
    , mMasterDisplay(nullptr)
    , mContext(None)
    , mDrawable(None)
    , mDummyVAO(GL_NONE)
    , mFBO(GL_NONE)
{
}

Backbuffer::~Backbuffer()
{
    glDeleteFramebuffers(1, &mFBO);
    glDeleteTextures(1, &mTexture);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &mDummyVAO);
    glXMakeCurrent(mMasterDisplay, None, 0);
    glXDestroyContext(mMasterDisplay, mContext);
    mMasterDisplay = nullptr;
    mWindow = 0;
}

bool Backbuffer::Resize(int newWidth, int newHeight)
{
    NFE_UNUSED(newWidth);
    NFE_UNUSED(newHeight);

    return 0;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    // get window and connect to a display
    mWindow = reinterpret_cast<Window>(desc.windowHandle);

    const MasterContext& ctx = MasterContext::Instance();

    mMasterDisplay = ctx.mDisplay;
    if (!mMasterDisplay)
    {
        NFE_LOG_ERROR("Master Context is uninitialized! No connection to X server");
        return false;
    }

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

        mContext = glXCreateContextAttribsARB(mMasterDisplay, ctx.mBestFB,
                                              ctx.mContext, GL_TRUE, attribs);
        if (!mContext)
        {
            LOG_WARNING("GL 3.3 or newer not acquired. Falling back to old one.");
            LOG_WARNING("Keep in mind, the renderer MIGHT NOT WORK due to too old OGL version!");
            // failed, fallback to classic method
            mContext = glXCreateNewContext(mMasterDisplay, ctx.mBestFB,
                                           GLX_RGBA_TYPE, ctx.mContext, GL_TRUE);
            if (!mContext)
            {
                NFE_LOG_ERROR("Cannot create OpenGL Context.");
                return false;
            }
        }
    }
    else
    {
        LOG_WARNING("glXCreateContextAttribsARB not available. Creating OGL context the old way.");
        LOG_WARNING("Keep in mind, the renderer MIGHT NOT WORK due to too old OGL version!");
        mContext = glXCreateNewContext(mMasterDisplay, ctx.mBestFB,
                                       GLX_RGBA_TYPE, ctx.mContext, GL_TRUE);
    }
    glXMakeCurrent(mMasterDisplay, mWindow, mContext);
    mDrawable = glXGetCurrentDrawable();

    // For information purposes - print what OpenGL Context we achieved
    const GLubyte* glv = glGetString(GL_VERSION);
    const char* glvStr = reinterpret_cast<const char*>(glv);
    bool direct = glXIsDirect(mMasterDisplay, mContext);
    NFE_LOG_INFO("OpenGL %s %s Slave Context obtained.", glvStr,
             direct ? "Direct" : "Indirect");

    // some systems might have glXSwapIntervalEXT unavailable
    if (glXSwapIntervalEXT)
        glXSwapIntervalEXT(mMasterDisplay, mDrawable, desc.vSync);
    else
        LOG_WARNING("glXSwapIntervalEXT was not acquired, VSync control is disabled.");

    CreateCommonResources(desc);

    return true;
}

bool Backbuffer::Present()
{
    BlitFramebuffers();

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
