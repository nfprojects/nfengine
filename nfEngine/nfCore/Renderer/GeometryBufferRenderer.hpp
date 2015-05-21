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
    void SetTarget(RenderContext *pContext, IRenderTarget* pTarget);
    void SetCamera(RenderContext *pContext, const CameraRenderDesc* pCamera);
    void SetMaterial(RenderContext *pContext, const RendererMaterial* pMaterial);
    void Draw(RenderContext *pContext, const RenderCommandBuffer& buffer);
};

} // namespace Renderer
} // namespace NFE
