/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definition of renderer's Backbuffer and its common functions
 */

#include "PCH.hpp"
#include "Backbuffer.hpp"


namespace NFE {
namespace Renderer {

void Backbuffer::CreateCommonResources(const BackbufferDesc& desc)
{
    // On Linux using OGL Core Profile a VAO must be bound to the pipeline for rendering. It can
    // be any VAO, however without it no drawing is performed.
    // Since VAOs are not shared between contexts, it is best to assume that each context
    // will create its own VAO for use.
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
    mGLTarget = GL_TEXTURE_2D;

    // Bind current texture to its own FBO
    // This way, the texture will be shared between two FBOs - drawing (set by RenderTarget class)
    // and reading (this one). We will be able to Blit the texture to a window when Present()
    // is called.
    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           mTexture, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void Backbuffer::BlitFramebuffers()
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
}

} // namespace Renderer
} // namespace NFE
