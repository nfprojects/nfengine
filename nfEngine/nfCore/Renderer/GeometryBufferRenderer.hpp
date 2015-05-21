/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Geometry Buffer Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "RenderCommand.hpp"

namespace NFE {
namespace Renderer {

struct GBufferRendererData
{
    const RendererMaterial* mCurrMaterial;
    InstanceData* mInstanceData;
};

class GBufferRenderer : public RendererModule<GBufferRenderer>
{
public:
    void SetTarget(RenderContext* context, IRenderTarget* target);
    void SetCamera(RenderContext* context, const CameraRenderDesc* camera);
    void SetMaterial(RenderContext* context, const RendererMaterial* material);
    void Draw(RenderContext* context, const RenderCommandBuffer& buffer);
};

} // namespace Renderer
} // namespace NFE
