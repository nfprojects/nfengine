/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Postprocess Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "PostProcessRendererContext.hpp"
#include "MultiPipelineState.hpp"


namespace NFE {
namespace Renderer {

struct ToneMappingParameters
{
    float saturation;
    float noiseFactor;
    float exposureOffset;
};

class PostProcessRenderer : public RendererModule<PostProcessRenderer, PostProcessRendererContext>
{
    // TODO: these are common with lights renderer:
    VertexLayoutPtr mVertexLayout;
    BufferPtr mVertexBuffer;

    MultiPipelineState mTonemappingPipelineState;
    BufferPtr mTonemappingCBuffer;

    ResourceBindingSetPtr mTexturesBindingSet;
    ResourceBindingLayoutPtr mResBindingLayout;
    ResourceBindingInstancePtr mNullTextureBindingInstance;

    bool CreateResourceBindingLayouts();

public:
    PostProcessRenderer();
    void OnEnter(PostProcessRendererContext* context) override;
    void OnLeave(PostProcessRendererContext* context) override;

    /**
     * Create shader resource binding for a texture that will be input for a postprocess
     * pass.
     */
    ResourceBindingInstancePtr CreateTextureBinding(const TexturePtr& texture);

    /**
     * Apply tonemapping, gamma correction and dithering (final post-process).
     */
    void ApplyTonemapping(PostProcessRendererContext* context,
                          const ToneMappingParameters& params,
                          ResourceBindingInstancePtr src,
                          RenderTargetPtr dest);
};

} // namespace Renderer
} // namespace NFE
