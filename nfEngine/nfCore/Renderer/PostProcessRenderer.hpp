/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Postprocess Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "PostProcessRendererContext.hpp"
#include "../Resources/MultiPipelineState.hpp"

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
    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IBuffer> mVertexBuffer;

    Resource::MultiPipelineState mTonemappingPipelineState;
    std::unique_ptr<IBuffer> mTonemappingCBuffer;

    std::unique_ptr<IResourceBindingSet> mTexturesBindingSet;
    std::unique_ptr<IResourceBindingLayout> mResBindingLayout;
    std::unique_ptr<IResourceBindingInstance> mNullTextureBindingInstance;

    bool CreateResourceBindingLayouts();

public:
    PostProcessRenderer();
    void OnEnter(PostProcessRendererContext* context) override;
    void OnLeave(PostProcessRendererContext* context) override;

    /**
     * Create shader resource binding for a texture that will be input for a postprocess
     * pass.
     */
    std::unique_ptr<IResourceBindingInstance> CreateTextureBinding(ITexture* texture);

    /**
     * Apply tonemapping, gamma correction and dithering (final post-process).
     */
    void ApplyTonemapping(PostProcessRendererContext* context, const ToneMappingParameters& params,
                          IResourceBindingInstance* src, IRenderTarget* dest);
};

} // namespace Renderer
} // namespace NFE
