/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Geometry Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "RenderCommand.hpp"
#include "GeometryRendererContext.hpp"
#include "RendererResources.hpp"
#include "../Resources/Multishader.hpp"

namespace NFE {
namespace Renderer {

class GeometryRenderer : public RendererModule<GeometryRenderer>
{
    Resource::Multishader* mVertexShader;
    Resource::Multishader* mPixelShader;

    Resource::Multishader* mShadowVertexShader;
    Resource::Multishader* mShadowPixelShader;

    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IRasterizerState> mRasterizerState;
    std::unique_ptr<IBuffer> mInstancesVertexBuffer;
    std::unique_ptr<IBuffer> mMaterialCBuffer;
    std::unique_ptr<IBuffer> mGlobalCBuffer;
    std::unique_ptr<IBuffer> mShadowGlobalCBuffer;

    uint32 mUseMotionBlurMacroVS;
    uint32 mUseMotionBlurMacroPS;

public:
    GeometryRenderer();

    void OnEnter(RenderContext* context);
    void OnLeave(RenderContext* context);

    /**
     * Prepare for Geometry Buffer rendering.
     */
    void SetUp(RenderContext* context, GeometryBuffer* geometryBuffer,
               const CameraRenderDesc* cameraDesc);

    /**
     * Prepare for Shadow Map rendering.
     */
    void SetUpForShadowMap(RenderContext* context, ShadowMap* shadowMap,
                           const ShadowCameraRenderDesc* cameraDesc, uint32 faceID = 0);

    void SetMaterial(RenderContext* context, const RendererMaterial* material);
    void Draw(RenderContext* context, const RenderCommandBuffer& buffer);
};

} // namespace Renderer
} // namespace NFE
