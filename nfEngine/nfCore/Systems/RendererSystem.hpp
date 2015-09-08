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

    void RenderLights(Renderer::RenderContext* ctx) const;
    void RenderLightsDebug(Renderer::RenderContext* ctx) const;

    void RenderGBuffer(Renderer::RenderContext* ctx, CameraComponent* camera,
                       TransformComponent* cameraTransform);
    void RenderShadow(Renderer::RenderContext * ctx, LightComponent* pLight, uint32 faceID = 0);
    void FindActiveMeshEntities();
    void FindVisibleMeshEntities(const Math::Frustum & frustum, std::vector<MeshEntry>& list);
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
