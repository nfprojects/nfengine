/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Shadows Renderer
 */

#pragma once

#include "Multishader.hpp"
#include "Renderer.hpp"
#include "RenderCommand.hpp"

namespace NFE {
namespace Renderer {

struct ShadowRendererData
{
};

// TODO: remove inheritance
class ShadowRenderer
{
public:
    void Release();
    void Enter(NFE_CONTEXT_ARG);
    void Leave(NFE_CONTEXT_ARG);

    void SetDestination(NFE_CONTEXT_ARG, const CameraRenderDesc* pCamera, ShadowMap* pShadowMap,
                        uint32 faceID);
    void SetMaterial(NFE_CONTEXT_ARG, const RendererMaterial* pMaterial);
    void Draw(NFE_CONTEXT_ARG, const RenderCommandBuffer& buffer);
};

} // namespace Renderer
} // namespace NFE
