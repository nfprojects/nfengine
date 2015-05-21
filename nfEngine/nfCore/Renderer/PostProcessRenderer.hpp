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
    int ApplyAntialiasing(RenderContext* context, ITexture* source, IRenderTarget* dest);
    int ApplyFXAA(RenderContext* context, const FXAADesc& desc);

    int ApplyTonemapping(RenderContext* context, const ToneMappingDesc& desc);
    int Downsaple(RenderContext* context, uint32 srcWidth, uint32 srcHeight, ITexture* source,
                  IRenderTarget* dest);
    int Blur(RenderContext* context, uint32 srcWidth, uint32 srcHeight, ITexture* source,
             IRenderTarget* dest, UINT Mode);
    int AverageTexture(RenderContext* context, ITexture* pSource, uint32 width, uint32 height);
    float GetAverageColor(RenderContext* context, uint32 width, uint32 height);

    int ApplyMotionBlur(RenderContext* context, uint32 srcWidth, uint32 srcHeight,
                        ITexture* source, IRenderTarget* dest, float dt);
};

} // namespace Renderer
} // namespace NFE
