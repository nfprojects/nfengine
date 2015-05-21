/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level Postprocess Renderer
 */

#pragma once

#include "../PCH.hpp"
#include "PostProcessRenderer.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<PostProcessRenderer> PostProcessRenderer::mPtr;


int PostProcessRenderer::ApplyAntialiasing(RenderContext* context, ITexture* source,
                                           IRenderTarget* dest)
{
    return 0;
}

int PostProcessRenderer::ApplyFXAA(RenderContext* context, const FXAADesc& desc)
{
    return 0;
}

int PostProcessRenderer::ApplyTonemapping(RenderContext* context, const ToneMappingDesc& desc)
{
    return 0;
}

int PostProcessRenderer::Downsaple(RenderContext* context, uint32 srcWidth, uint32 srcHeight,
                                   ITexture* source, IRenderTarget* dest)
{
    return 0;
}

int PostProcessRenderer::Blur(RenderContext* context, uint32 srcWidth, uint32 srcHeight,
                              ITexture* source, IRenderTarget* dest, UINT Mode)
{
    return 0;
}

int PostProcessRenderer::AverageTexture(RenderContext* context, ITexture* source,
                                        uint32 width, uint32 height)
{
    return 0;
}

float PostProcessRenderer::GetAverageColor(RenderContext* context, uint32 width, uint32 height)
{
    return 0;
}

int PostProcessRenderer::ApplyMotionBlur(RenderContext* context, uint32 srcWidth, uint32 srcHeight,
                                         ITexture* source, IRenderTarget* dest, float dt)
{
    return 0;
}

} // namespace Renderer
} // namespace NFE
