/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Postprocess Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"

namespace NFE {
namespace Renderer {

class PostProcessRenderer : public RendererModule<PostProcessRenderer>
{
public:
    int ApplyAntialiasing(RenderContext *pContext, ITexture* pSource, IRenderTarget* pDestination);
    int ApplyFXAA(RenderContext *pContext, const FXAADesc& desc);

    int ApplyTonemapping(RenderContext *pContext, const ToneMappingDesc& desc);
    int Downsaple(RenderContext *pContext, uint32 srcWidth, uint32 srcHeight, ITexture* pSource,
                  IRenderTarget* pDest);
    int Blur(RenderContext *pContext, uint32 srcWidth, uint32 srcHeight, ITexture* pSource,
             IRenderTarget* pDest, UINT Mode);
    int AverageTexture(RenderContext *pContext, ITexture* pSource, uint32 width, uint32 height);
    float GetAverageColor(RenderContext *pContext, uint32 width, uint32 height);

    int ApplyMotionBlur(RenderContext *pContext, uint32 srcWidth, uint32 srcHeight,
                        ITexture* pSource, IRenderTarget* pDestination, float dt);
};

} // namespace Renderer
} // namespace NFE
