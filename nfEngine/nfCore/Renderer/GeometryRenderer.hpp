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
#include "../Resources/MultiShaderProgram.hpp"

namespace NFE {
namespace Renderer {

class GeometryRenderer : public RendererModule<GeometryRenderer>
{
    Resource::MultiShaderProgram mGeometryPassShaderProgram;
    Resource::MultiShaderProgram mShadowShaderProgram;

    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IPipelineState> mPipelineState;
    std::unique_ptr<IBuffer> mInstancesVertexBuffer;
    std::unique_ptr<IBuffer> mMaterialCBuffer;
    std::unique_ptr<IBuffer> mGlobalCBuffer;
    std::unique_ptr<IBuffer> mShadowGlobalCBuffer;

    std::unique_ptr<IResourceBindingSet> mGlobalBindingSet;
    std::unique_ptr<IResourceBindingSet> mMatCBufferBindingSet;
    std::unique_ptr<IResourceBindingSet> mMatTexturesBindingSet;
    std::unique_ptr<IResourceBindingLayout> mResBindingLayout;
    std::unique_ptr<IResourceBindingInstance> mGlobalBindingInstance;
    std::unique_ptr<IResourceBindingInstance> mShadowGlobalBindingInstance;
    std::unique_ptr<IResourceBindingInstance> mMatCBufferBindingInstance;

    uint32 mUseMotionBlurMacroId;
    uint32 mCubeShadowMapMacroId;

    bool CreateResourceBindingLayouts();

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
