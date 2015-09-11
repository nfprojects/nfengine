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

NFE_ALIGN16
class RendererSystem : public Util::Aligned
{
    typedef std::tuple<TransformComponent*, MeshComponent*, BodyComponent*> MeshEntry;
    typedef std::tuple<TransformComponent*, LightComponent*> LightEntry;
    typedef std::tuple<TransformComponent*, CameraComponent*, BodyComponent*> CameraEntry;

    std::vector<LightEntry> mOmniLights;
    std::vector<LightEntry> mSpotLights;
    std::vector<LightEntry> mDirLights;
    std::vector<SpotLightData, Util::AlignedAllocator<SpotLightData, 16>> mSpotLightsData;

    std::vector<MeshEntry> mMeshes;
    std::vector<MeshEntry> mActiveMeshEntities;
    std::vector<CameraEntry> mCameras;

    SceneManager* mScene;

    void RenderShadowMaps() const;
    void RenderLights(Renderer::RenderContext* ctx) const;
    void RenderLightsDebug(Renderer::RenderContext* ctx) const;

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
    void UpdateLights();

public:
    RendererSystem(SceneManager* scene);

    /**
     * Update system.
     */
    void Update(float dt);

    void Render(Renderer::View* view);
};

} // namespace Scene
} // namespace NFE
