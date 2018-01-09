/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   OGL4 implementation of renderer's backbuffer
 */

#include "../PCH.hpp"

#include "../Defines.hpp"
#include "../Backbuffer.hpp"
#include "../MasterContext.hpp"

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
    glDeleteFramebuffers(1, &mFBO);
    glDeleteTextures(1, &mTexture);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &mDummyVAO);

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
    NFE_UNUSED(newWidth);
    NFE_UNUSED(newHeight);

    return 0;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    mType = TextureType::Texture2D;
    mHWND = static_cast<HWND>(desc.windowHandle);
    mWidth = desc.width;
    mHeight = desc.height;
    const MasterContext& mc = MasterContext::Instance();

    // Initialize OpenGL
    mHDC = GetDC(mHWND);

    int pixelFormat = 0;
    unsigned int numFormats = 0;
    if (!wglChoosePixelFormatARB(mHDC, mc.mPixelFormatIntAttribs, mc.mPixelFormatFloatAttribs,
                                 1, &pixelFormat, &numFormats))
    {
        NFE_LOG_ERROR("Failed to choose a Pixel Format for Slave Context.");
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    if (!SetPixelFormat(mHDC, pixelFormat, &pfd))
    {
        NFE_LOG_ERROR("Failed to set pixel format.");
        return false;
    }

    mHRC = wglCreateContextAttribsARB(mHDC, mc.mHRC, mc.mAttribs);
    if (!mHRC)
    {
        NFE_LOG_ERROR("Failed to create Slave Context.");
        return false;
    }

    if (!wglMakeCurrent(mHDC, mHRC))
    {
        NFE_LOG_ERROR("Failed to make Core Context current.");
        return false;
    }

    // enable/disable vsync
    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(desc.vSync);
    else
        LOG_WARNING("wglSwapIntervalEXT was not acquired, VSync control is disabled.");

    CreateCommonResources(desc);

    const GLubyte* glv = glGetString(GL_VERSION);
    const char* glvStr = reinterpret_cast<const char*>(glv);
    NFE_LOG_INFO("OpenGL %s Slave Context obtained.", glvStr);

    return true;
}

bool Backbuffer::Present()
{
    BlitFramebuffers();

    // OGL wiki recommends following scheme of work:
    //   * VSync enabled - just swap buffers, we can buy us some CPU work that way
    //   * VSync disabled - call glFinish before swap to make sure all calls are done

    // VSync is enabled if wglGetSwapIntervalEXT returns 1 or higher
    if (wglGetSwapIntervalEXT && !wglGetSwapIntervalEXT())
        glFinish();

    if (SwapBuffers(mHDC))
        return false;
    else
        return true;
}

} // namespace Renderer
} // namespace NFE
