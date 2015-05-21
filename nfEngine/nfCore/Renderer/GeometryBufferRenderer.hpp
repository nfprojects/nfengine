/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Geometry Buffer Renderer
 */

#pragma once

#include "Renderer.hpp"
#include "RenderCommand.hpp"

namespace NFE {
namespace Renderer {

struct GBufferRendererData
{
    const RendererMaterial* mCurrMaterial;
    InstanceData* mInstanceData;
};

class GBufferRenderer
{
public:
    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    void SetTarget(NFE_CONTEXT_ARG, IRenderTarget* pTarget);
    void SetCamera(NFE_CONTEXT_ARG, const CameraRenderDesc* pCamera);
    void SetMaterial(NFE_CONTEXT_ARG, const RendererMaterial* pMaterial);
    void Draw(NFE_CONTEXT_ARG, const RenderCommandBuffer& buffer);
};

} // namespace Renderer
} // namespace NFE
