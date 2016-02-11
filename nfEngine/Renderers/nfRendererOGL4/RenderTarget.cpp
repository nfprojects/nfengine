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
{
}

RenderTarget::~RenderTarget()
{
    glDeleteFramebuffers(1, &mFBO);
}

void RenderTarget::GetDimensions(int& width, int& height)
{
    UNUSED(width);
    UNUSED(height);
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    // TODO this limitation must be removed
    if (desc.numTargets > 1)
    {
        LOG_ERROR("Multiple targets are not supported!");
        return false;
    }

    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);

    if (desc.numTargets > 0)
    {
        Texture* tex = dynamic_cast<Texture*>(desc.targets[0].texture);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               tex->mTexture, 0);
    }

    if (desc.depthBuffer)
    {
        Texture* depthTex = dynamic_cast<Texture*>(desc.depthBuffer);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                               depthTex->mTexture, 0);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return true;
}

} // namespace Renderer
} // namespace NFE
