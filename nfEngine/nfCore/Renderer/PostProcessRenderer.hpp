/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Postprocess Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "../Resources/MultiShaderProgram.hpp"

namespace NFE {
namespace Renderer {

struct ToneMappingParameters
{
    float saturation;
    float noiseFactor;
    float exposureOffset;
};

class PostProcessRenderer : public RendererModule<PostProcessRenderer>
{
    // TODO: these are common with lights renderer:
    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IBuffer> mVertexBuffer;

    Resource::MultiShaderProgram mTonemappingShaderProgram;
    std::unique_ptr<IBuffer> mTonemappingCBuffer;

public:
    PostProcessRenderer();
    void OnEnter(RenderContext* context);
    void OnLeave(RenderContext* context);

    /**
     * Apply tonemapping, gamma correction and dithering (final post-process).
     */
    void ApplyTonemapping(RenderContext* context, const ToneMappingParameters& params,
                          ITexture* src, IRenderTarget* dest);
};

} // namespace Renderer
} // namespace NFE
