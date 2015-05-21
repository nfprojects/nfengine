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
    bool ApplyAntialiasing(RenderContext* context, ITexture* source, IRenderTarget* dest);
    bool ApplyFXAA(RenderContext* context, const FXAADesc& desc);

    bool ApplyTonemapping(RenderContext* context, const ToneMappingDesc& desc);
    bool Downsaple(RenderContext* context, uint32 srcWidth, uint32 srcHeight, ITexture* source,
                  IRenderTarget* dest);
    bool Blur(RenderContext* context, uint32 srcWidth, uint32 srcHeight, ITexture* source,
              IRenderTarget* dest, UINT Mode);
    bool AverageTexture(RenderContext* context, ITexture* pSource, uint32 width, uint32 height);
    float GetAverageColor(RenderContext* context, uint32 width, uint32 height);

    bool ApplyMotionBlur(RenderContext* context, uint32 srcWidth, uint32 srcHeight,
                         ITexture* source, IRenderTarget* dest, float dt);
};

} // namespace Renderer
} // namespace NFE
