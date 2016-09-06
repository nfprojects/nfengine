/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system.
 */

#pragma once

#include "../Core.hpp"
#include "nfCommon/Aligned.hpp"
#include "nfCommon/ThreadPool.hpp"
#include "nfCommon/Math/Frustum.hpp"
#include "nfCommon/BVH.hpp"
#include "../Renderer/RendererResources.hpp"
#include "../Renderers/RendererInterface/CommandBuffer.hpp"
#include "../Scene/EntityManager.hpp"

namespace NFE {
namespace Scene {

struct MeshEntry
{
    TransformComponent* transform;
    MeshComponent* mesh;
    BodyComponent* body;

    uint32 bvhNode;

    // TODO: keep more renderer variables: AABB, BVH node, etc.
};

struct NFE_ALIGN16 LightEntry
{
    Math::Frustum frustum;
    Math::Matrix viewMatrix;
    Math::Matrix projMatrix;

    TransformComponent* transform;
    LightComponent* light;

    std::atomic_bool shadowDrawn;


    // required because of atomic member
    NFE_INLINE LightEntry& operator= (const LightEntry& other)
    {
        frustum = other.frustum;
        viewMatrix = other.viewMatrix;
        projMatrix = other.projMatrix;
        transform = other.transform;
        light = other.light;
        shadowDrawn = false;

        return *this;
    }

    // TODO: keep more renderer variables: AABB, BVH node, etc.
};

/**
 * Temporary data used during single View rendering.
 */
 // TODO: there are "false sharing" problems here
struct NFE_ALIGN16 RenderingData
{
    typedef std::unique_ptr<Renderer::ICommandList> CommandListPtr;

    Renderer::View* view;
    CameraComponent* cameraComponent;
    TransformComponent* cameraTransform;
    Renderer::CameraRenderDesc cameraRenderDesc;

    std::vector<LightEntry*> visibleOmniLights;
    std::vector<LightEntry*> visibleSpotLights;

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
    RenderingData(const RenderingData& other);
    void ExecuteCommandLists() const;
};

NFE_ALIGN16
class RendererSystem : public Common::Aligned<16>
{
    using LightEntries = std::map<EntityID, LightEntry, Common::AlignedAllocator<LightEntry, 16>>;

    typedef std::tuple<TransformComponent*, CameraComponent*, BodyComponent*> CameraEntry;

    Common::TaskID mUpdateTask;

    EntityListener mMeshesListener;
    EntityListener mLightsListener;

    Common::BVH mMeshesBVH;

    /**
     * Temorary, per-frame data (they are overwritten by Update method).
     * Note that they can be shared by multiple Views.
     */
    std::unordered_map<EntityID, MeshEntry> mMeshes;      // active mesh entities
    std::vector<CameraEntry> mCameras;          // active camera entities
    std::unordered_map<EntityID, LightEntry> mLights;                       // active light entities


    // which spot lights have its shadow map already rendered?
    std::unique_ptr<std::atomic_bool[]> mSpotLightsShadowDrawn;
    // which omni lights have its shadow map already rendered?
    std::unique_ptr<std::atomic_bool[]> mOmniLightsShadowDrawn;

    SceneManager* mScene;

    void RenderDebugLayer(const Common::TaskContext& context, RenderingData& data) const;
    void RenderLightsDebug(RenderingData& data, Renderer::RenderContext* ctx) const;
    void RenderShadowMaps(const Common::TaskContext& context, RenderingData& data);
    void RenderSpotShadowMap(const Common::TaskContext& context,
                             const LightEntry* lightData,
                             RenderingData& data) const;
    void RenderOmniShadowMap(const Common::TaskContext& context,
                             const LightEntry* lightData,
                             RenderingData& data) const;
    void RenderLights(const Common::TaskContext& context, RenderingData& data) const;

    /**
     * Draw all the geometry visible in the @p viewFrustum using GeometryRenderer.
     * @param ctx             Renderer contex
     * @param viewFrustum     A frustum against which frustum culling will be performed
     * @param cameraTransform Transform component of the viewing camera.
                              Used for distance calculation.
     */
    void RenderGeometry(Renderer::RenderContext* ctx, const Math::Frustum& viewFrustum,
                        const TransformComponent* cameraTransform) const;

    void FindVisibleMeshEntities(const Math::Frustum & frustum,
                                 std::vector<MeshEntry>& list) const;
    void FindVisibleLights(const Math::Frustum & frustum, RenderingData& data);

    void UpdateCameras();
    void UpdateLights();

    void OnMeshEntityCreated(EntityID entity);
    void OnMeshEntityRemoved(EntityID entity);
    void OnMeshEntityChanged(EntityID entity);

    void OnLightEntityCreated(EntityID entity);
    void OnLightEntityRemoved(EntityID entity);
    void OnLightEntityChanged(EntityID entity);

public:
    RendererSystem(SceneManager* scene);
    ~RendererSystem();

    /**
     * Update the renderer system.
     * This function will be called as thread pool task.
     *
     * @param context  Thread pool's task context.
     * @param dt       Time delta in seconds.
     */
    void Update(const Common::TaskContext& context, float dt);

    /**
     * Render the scene for a view.
     * This function will be called as thread pool task.
     *
     * @param context       Thread pool's task context.
     * @param renderingData Temporary data used for the view rendering.
     */
    void Render(const Common::TaskContext& context, RenderingData& renderingData);
};

} // namespace Scene
} // namespace NFE
