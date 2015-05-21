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
    void SetDestination(RenderContext* context, const CameraRenderDesc* camera,
                        ShadowMap* shadowMap, uint32 faceID);
    void SetMaterial(RenderContext* context, const RendererMaterial* material);
    void Draw(RenderContext* context, const RenderCommandBuffer& buffer);
};

} // namespace Renderer
} // namespace NFE
