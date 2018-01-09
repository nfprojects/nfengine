/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   OGL4 implementation of renderer's render target
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"


namespace NFE {
namespace Renderer {

RenderTarget::RenderTarget()
    : mFBO(GL_NONE)
    , mWidth(0)
    , mHeight(0)
{
}

RenderTarget::~RenderTarget()
{
    glDeleteFramebuffers(1, &mFBO);
}

void RenderTarget::GetDimensions(int& width, int& height)
{
    width = mWidth;
    height = mHeight;
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    glGenFramebuffers(1, &mFBO);

    int boundFBO = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &boundFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);

    if (desc.numTargets > 0)
    {
        mAttachments.resize(desc.numTargets);

        for (unsigned int i = 0; i < desc.numTargets; ++i)
        {
            Texture* tex = dynamic_cast<Texture*>(desc.targets[i].texture);
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, tex->mGLTarget,
                                   tex->mTexture, 0);
            mAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
        }
    }

    if (desc.depthBuffer)
    {
        Texture* depthTex = dynamic_cast<Texture*>(desc.depthBuffer);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
                               depthTex->mHasStencil
                                   ? GL_DEPTH_STENCIL_ATTACHMENT
                                   : GL_DEPTH_ATTACHMENT,
                               depthTex->mGLTarget,
                               depthTex->mTexture, 0);

        // D3D requires same size for all buffers bound to Render Target,
        // so we can assume the sizes match and extract width/height duo here
        mWidth = depthTex->mWidth;
        mHeight = depthTex->mHeight;
    }

    GLenum status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        NFE_LOG_ERROR("Framebuffer Object is incomplete! Status: 0x%x", status);
        return false;
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, boundFBO);

    return true;
}

} // namespace Renderer
} // namespace NFE
