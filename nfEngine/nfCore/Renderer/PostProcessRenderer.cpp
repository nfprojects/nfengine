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

void PostProcessRenderer::Enter(NFE_CONTEXT_ARG)
{
}

void PostProcessRenderer::Leave(NFE_CONTEXT_ARG)
{
}

int PostProcessRenderer::ApplyAntialiasing(NFE_CONTEXT_ARG, ITexture* pSource,
                                           IRenderTarget* pDestination)
{
    return 0;
}

int PostProcessRenderer::ApplyFXAA(NFE_CONTEXT_ARG, const FXAADesc& desc)
{
    return 0;
}

int PostProcessRenderer::ApplyTonemapping(NFE_CONTEXT_ARG, const ToneMappingDesc& desc)
{
    return 0;
}

int PostProcessRenderer::Downsaple(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight,
                                   ITexture* pSource, IRenderTarget* pDest)
{
    return 0;
}

int PostProcessRenderer::Blur(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight,
                              ITexture* pSource, IRenderTarget* pDest, UINT Mode)
{
    return 0;
}

int PostProcessRenderer::AverageTexture(NFE_CONTEXT_ARG, ITexture* pSource,
                                        uint32 width, uint32 height)
{
    return 0;
}

float PostProcessRenderer::GetAverageColor(NFE_CONTEXT_ARG, uint32 width, uint32 height)
{
    return 0;
}

int PostProcessRenderer::ApplyMotionBlur(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight,
                                         ITexture* pSource, IRenderTarget* pDestination,
                                         float dt)
{
    return 0;
}

} // namespace Renderer
} // namespace NFE
