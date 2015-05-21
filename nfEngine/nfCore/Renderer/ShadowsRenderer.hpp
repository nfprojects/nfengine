/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Shadows Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "RenderCommand.hpp"

namespace NFE {
namespace Renderer {

struct ShadowRendererData
{
};

class ShadowRenderer : public RendererModule<ShadowRenderer>
{
public:
    void SetDestination(RenderContext *pContext, const CameraRenderDesc* pCamera, ShadowMap* pShadowMap,
                        uint32 faceID);
    void SetMaterial(RenderContext *pContext, const RendererMaterial* pMaterial);
    void Draw(RenderContext *pContext, const RenderCommandBuffer& buffer);
};

} // namespace Renderer
} // namespace NFE
