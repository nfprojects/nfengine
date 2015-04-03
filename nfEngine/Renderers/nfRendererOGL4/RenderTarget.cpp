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
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    UNUSED(desc);
    return true;
}

} // namespace Renderer
} // namespace NFE
