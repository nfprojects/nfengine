/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of high-level Geometry Renderer
 */

#pragma once

#include "RendererModule.hpp"
#include "RenderCommand.hpp"
#include "GeometryRendererContext.hpp"
#include "RendererResources.hpp"
#include "MultiPipelineState.hpp"


namespace NFE {
namespace Renderer {

class GeometryRenderer : public RendererModule<GeometryRenderer, GeometryRendererContext>
{
    MultiPipelineState mGeometryPassPipelineState;
    MultiPipelineState mShadowPipelineState;

    VertexLayoutPtr mVertexLayout;
    BufferPtr mInstancesVertexBuffer;
    BufferPtr mMaterialCBuffer;
    BufferPtr mGlobalCBuffer;
    BufferPtr mShadowGlobalCBuffer;

    ResourceBindingSetPtr mMatTexturesBindingSet;
    ResourceBindingLayoutPtr mResBindingLayout;
    ResourceBindingInstancePtr mGlobalBindingInstance;
    ResourceBindingInstancePtr mDummyMaterialBindingInstance;

    uint32 mUseMotionBlurMacroId;
    uint32 mCubeShadowMapMacroId;

    bool CreateResourceBindingLayouts();

public:
    GeometryRenderer();

    void OnEnter(GeometryRendererContext* context) override;
    void OnLeave(GeometryRendererContext* context) override;

    NFE_INLINE const ResourceBindingSetPtr& GetMaterialTexturesBindingSet() const
    {
        return mMatTexturesBindingSet;
    }

    /**
     * Prepare for Geometry Buffer rendering.
     */
    void SetUp(GeometryRendererContext* context, GeometryBuffer* geometryBuffer,
               const CameraRenderDesc* cameraDesc);

    /**
     * Prepare for Shadow Map rendering.
     */
    void SetUpForShadowMap(GeometryRendererContext* context, ShadowMap* shadowMap,
                           const ShadowCameraRenderDesc* cameraDesc, uint32 faceID = 0);

    void SetMaterial(GeometryRendererContext* context, const RendererMaterial* material);
    void Draw(GeometryRendererContext* context, const RenderCommandBuffer& buffer);
};

} // namespace Renderer
} // namespace NFE
