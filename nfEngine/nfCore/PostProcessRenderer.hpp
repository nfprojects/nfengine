/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Postprocess Renderer
 */

#pragma once

#include "Renderer.hpp"
#include "Multishader.hpp"

namespace NFE {
namespace Renderer {

class PostProcessRenderer
{
public:
    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    int ApplyAntialiasing(NFE_CONTEXT_ARG, ITexture* pSource, IRenderTarget* pDestination);
    int ApplyFXAA(NFE_CONTEXT_ARG, const FXAADesc& desc);

    int ApplyTonemapping(NFE_CONTEXT_ARG, const ToneMappingDesc& desc);
    int Downsaple(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight, ITexture* pSource,
                  IRenderTarget* pDest);
    int Blur(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight, ITexture* pSource,
             IRenderTarget* pDest, UINT Mode);
    int AverageTexture(NFE_CONTEXT_ARG, ITexture* pSource, uint32 width, uint32 height);
    float GetAverageColor(NFE_CONTEXT_ARG, uint32 width, uint32 height);

    int ApplyMotionBlur(NFE_CONTEXT_ARG, uint32 srcWidth, uint32 srcHeight,
                        ITexture* pSource, IRenderTarget* pDestination, float dt);
};

} // namespace Renderer
} // namespace NFE
