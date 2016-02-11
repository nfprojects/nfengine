/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   OGL4 implementation of renderer's render target
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "RenderTarget.hpp"


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
    if (desc.numTargets > 1)
    {
        LOG_ERROR("Multiple targets are not supported!");
        return false;
    }

    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
    //glFramebufferTexture2D(GL_READ_FRAMEBUFFER, )
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return true;
}

} // namespace Renderer
} // namespace NFE
