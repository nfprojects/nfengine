/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system.
 */

#pragma once

#include "../Core.hpp"
#include "../Aligned.hpp"
#include "../nfCommon/ThreadPool.hpp"
#include "../nfCommon/Math/Frustum.hpp"
#include "../Renderer/RendererResources.hpp"
#include "../Renderers/RendererInterface/CommandBuffer.hpp"

namespace NFE {
namespace Scene {

/**
 * Temporary data of active spot light.
 */
struct NFE_ALIGN16 SpotLightData
{
    Math::Frustum frustum;
    Math::Matrix viewMatrix;
    Math::Matrix projMatrix;
};

/**
 * Temporary data used during view rendering.
 */
struct RenderingData
{
    typedef std::unique_ptr<Renderer::ICommandList> CommandListPtr;

    Renderer::View* view;
    CameraComponent* cameraComponent;
    TransformComponent* cameraTransform;
    Renderer::CameraRenderDesc cameraRenderDesc;

    std::vector<uint32> visibleOmniLights;
    std::vector<uint32> visibleSpotLights;

    Common::TaskID sceneRenderTask;

    Common::TaskID shadowPassTask;
    // list of shadow command lists for each thread
    std::vector<std::vector<CommandListPtr>> shadowPassCLs;

    Common::TaskID geometryPassTask;
    CommandListPtr geometryPassCL;

    Common::TaskID lightsPassTask;
    CommandListPtr lightsPassCL;

    Common::TaskID debugLayerTask;
    CommandListPtr debugLayerCL;

    RenderingData();
    void ExecuteCommandLists() const;
};

NFE_ALIGN16
class RendererSystem : public Util::Aligned
{
    typedef std::tuple<TransformComponent*, MeshComponent*, BodyComponent*> MeshEntry;
    typedef std::tuple<TransformComponent*, LightComponent*> LightEntry;
    typedef std::tuple<TransformComponent*, CameraComponent*, BodyComponent*> CameraEntry;

    Common::TaskID mUpdateTask;

    std::vector<LightEntry> mOmniLights;
    std::vector<LightEntry> mSpotLights;
    std::vector<LightEntry> mDirLights;
    std::vector<SpotLightData, Util::AlignedAllocator<SpotLightData, 16>> mSpotLightsData;

    std::unique_ptr<std::atomic_bool[]> mSpotLightsShadowDrawn;
    std::unique_ptr<std::atomic_bool[]> mOmniLightsShadowDrawn;

    std::vector<MeshEntry> mMeshes;
    std::vector<MeshEntry> mActiveMeshEntities;
    std::vector<CameraEntry> mCameras;

    SceneManager* mScene;

    void DrawDebugLayer(const Common::TaskContext& context, RenderingData& data) const;

    void RenderSpotShadowMap(const Common::TaskContext& context,
                             const TransformComponent* transform,
                             const LightComponent* light,
                             const SpotLightData& lightData,
                             RenderingData& data) const;
    void RenderOmniShadowMap(const Common::TaskContext& context,
                             const TransformComponent* transform,
                             const LightComponent* light,
                             RenderingData& data) const;
    void RenderShadowMaps(const Common::TaskContext& context, RenderingData& data);
    void RenderLights(const Common::TaskContext& context, RenderingData& data) const;
    void RenderLightsDebug(RenderingData& data, Renderer::RenderContext* ctx) const;

    /**
     * Draw all the geometry visible in the @p viewFrustum using GeometryRenderer.
     * @param ctx             Renderer contex
     * @param viewFrustum     A frustum against which frustum culling will be performed
     * @param cameraTransform Transform component of the viewing camera.
                              Used for distance calculation.
     */
    void DrawGeometry(Renderer::RenderContext* ctx, const Math::Frustum& viewFrustum,
                      const TransformComponent* cameraTransform) const;

    void FindActiveMeshEntities();
    void FindVisibleMeshEntities(const Math::Frustum & frustum,
                                 std::vector<MeshEntry>& list) const;
    void FindVisibleLights(const Math::Frustum & frustum, RenderingData& data) const;

    void UpdateLights();

public:
    RendererSystem(SceneManager* scene);

    /**
     * Update system.
     */
    void Update(const Common::TaskContext& context, float dt);

    /**
     * Render the scene for a view.
     * This function will be called as thread pool task.
     * @param context       Thread pool's task context.
     * @param renderingData Temporary data used for the view rendering.
     */
    void Render(const Common::TaskContext& context, RenderingData& renderingData);
};

} // namespace Scene
} // namespace NFE
