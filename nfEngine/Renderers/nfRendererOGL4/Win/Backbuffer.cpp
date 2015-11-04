/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   OGL4 implementation of renderer's backbuffer
 */

#include "../PCH.hpp"

#include "../Defines.hpp"
#include "../Backbuffer.hpp"


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
    UNUSED(newWidth);
    UNUSED(newHeight);

    return 0;
}

bool Backbuffer::Init(const BackbufferDesc& desc)
{
    mType = TextureType::Texture2D;
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

    // TODO Proper creation of OpenGL context (wglCreateContext -> extract wglCreateContextAttribsARB)
    mHRC = wglCreateContext(mHDC);
    if (!mHRC)
        return false;

    wglMakeCurrent(mHDC, mHRC);

    // OpenGL Extensions can be initialized only after GL context is current
    if (!nfglExtensionsInit())
        return false;

    // enable/disable vsync
    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(desc.vSync);
    else
        LOG_WARNING("wglSwapIntervalEXT was not acquired, VSync control is disabled.");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    // To match OpenGL Core Profile behavior on Linux
    // See Linux/Backbuffer.cpp for more info
    glGenVertexArrays(1, &mDummyVAO);
    glBindVertexArray(mDummyVAO);

    // prepare a texture to draw to
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    // TODO match settings to default FBO
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, desc.width, desc.height, 0,
                 GL_RGBA, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    mWidth = desc.width;
    mHeight = desc.height;

    // Bind current texture to its own FBO
    // This way, the texture will be shared between two FBOs - drawing (set by RenderTarget class)
    // and reading (this one). We will be able to Blit the texture to a window when Present()
    // is called.
    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           mTexture, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return true;
}

bool Backbuffer::Present()
{
    // Acquire currently bound draw FBO
    // Eventual blitting to window FBO will require rebinding Framebuffers, so we need to remember
    // our original state.
    GLint boundDrawFBO = 0;
    GLint boundReadFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &boundDrawFBO);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &boundReadFBO);

    // Set the Framebuffers as we want them
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);

    // Do the copy
    glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Restore FBO state
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, boundDrawFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, boundReadFBO);

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
