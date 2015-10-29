/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Postprocess Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "Multishader.hpp"

namespace NFE {
namespace Renderer {

struct ToneMappingParameters
{
    float exposure;
};

class PostProcessRenderer : public RendererModule<PostProcessRenderer>
{
    // TODO: these are common with lights renderer:
    Multishader mFullscreenQuadVS;
    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IBuffer> mVertexBuffer;

    Multishader mTonemappingPS;
    std::unique_ptr<IBuffer> mTonemappingCBuffer;

public:
    PostProcessRenderer();
    void OnEnter(RenderContext* context);
    void OnLeave(RenderContext* context);

    bool ApplyTonemapping(RenderContext* context, const ToneMappingParameters& params,
                          ITexture* src, IRenderTarget* dest);
};

} // namespace Renderer
} // namespace NFE
