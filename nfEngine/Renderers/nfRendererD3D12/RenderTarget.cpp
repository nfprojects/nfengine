/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's render target
 */

#include "PCH.hpp"
#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "RendererD3D12.hpp"
#include "../../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

RenderTarget::RenderTarget()
    : mDepthBuffer(nullptr)
    , mWidth(0)
    , mHeight(0)
{
}

void RenderTarget::GetDimensions(int& width, int& height)
{
    width = mWidth;
    height = mHeight;
}

bool RenderTarget::Init(const RenderTargetDesc& desc)
{
    UNUSED(desc);
    return false;
}

} // namespace Renderer
} // namespace NFE
