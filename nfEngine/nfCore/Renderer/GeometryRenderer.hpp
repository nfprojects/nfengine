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
#include "../Resources/MultiPipelineState.hpp"

namespace NFE {
namespace Renderer {

class GeometryRenderer : public RendererModule<GeometryRenderer, GeometryRendererContext>
{
    Resource::MultiPipelineState mGeometryPassPipelineState;
    Resource::MultiPipelineState mShadowPipelineState;

    std::unique_ptr<IVertexLayout> mVertexLayout;
    std::unique_ptr<IBuffer> mInstancesVertexBuffer;
    std::unique_ptr<IBuffer> mMaterialCBuffer;
    std::unique_ptr<IBuffer> mGlobalCBuffer;
    std::unique_ptr<IBuffer> mShadowGlobalCBuffer;

    std::unique_ptr<IResourceBindingSet> mMatTexturesBindingSet;
    std::unique_ptr<IResourceBindingLayout> mResBindingLayout;
    std::unique_ptr<IResourceBindingInstance> mGlobalBindingInstance;
    std::unique_ptr<IResourceBindingInstance> mDummyMaterialBindingInstance;

    uint32 mUseMotionBlurMacroId;
    uint32 mCubeShadowMapMacroId;

    bool CreateResourceBindingLayouts();

public:
    GeometryRenderer();

    void OnEnter(GeometryRendererContext* context) override;
    void OnLeave(GeometryRendererContext* context) override;

    NFE_INLINE IResourceBindingSet* GetMaterialTexturesBindingSet() const
    {
        return mMatTexturesBindingSet.get();
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
