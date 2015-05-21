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


int PostProcessRenderer::ApplyAntialiasing(RenderContext *pContext, ITexture* pSource,
                                           IRenderTarget* pDestination)
{
    return 0;
}

int PostProcessRenderer::ApplyFXAA(RenderContext *pContext, const FXAADesc& desc)
{
    return 0;
}

int PostProcessRenderer::ApplyTonemapping(RenderContext *pContext, const ToneMappingDesc& desc)
{
    return 0;
}

int PostProcessRenderer::Downsaple(RenderContext *pContext, uint32 srcWidth, uint32 srcHeight,
                                   ITexture* pSource, IRenderTarget* pDest)
{
    return 0;
}

int PostProcessRenderer::Blur(RenderContext *pContext, uint32 srcWidth, uint32 srcHeight,
                              ITexture* pSource, IRenderTarget* pDest, UINT Mode)
{
    return 0;
}

int PostProcessRenderer::AverageTexture(RenderContext *pContext, ITexture* pSource,
                                        uint32 width, uint32 height)
{
    return 0;
}

float PostProcessRenderer::GetAverageColor(RenderContext *pContext, uint32 width, uint32 height)
{
    return 0;
}

int PostProcessRenderer::ApplyMotionBlur(RenderContext *pContext, uint32 srcWidth, uint32 srcHeight,
                                         ITexture* pSource, IRenderTarget* pDestination,
                                         float dt)
{
    return 0;
}

} // namespace Renderer
} // namespace NFE
