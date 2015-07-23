/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system.
 */

#pragma once

#include "../Core.hpp"
#include "../Aligned.hpp"
#include "../nfCommon/ThreadPool.hpp"

namespace NFE {
namespace Scene {

NFE_ALIGN16
class RendererSystem : public Util::Aligned
{
    typedef std::tuple<TransformComponent*, MeshComponent*, BodyComponent*> MeshEntry;
    typedef std::tuple<TransformComponent*, LightComponent*> LightEntry;
    typedef std::tuple<TransformComponent*, CameraComponent*, BodyComponent*> CameraEntry;

    std::vector<MeshEntry> mMeshes;
    std::vector<MeshEntry> mActiveMeshEntities;
    std::vector<LightEntry> mLights;
    std::vector<CameraEntry> mCameras;

    SceneManager* mScene;

    void RenderLight(Renderer::RenderContext* ctx, LightComponent* light, TransformComponent* transform);
    void RenderLightDebug(Renderer::RenderContext* ctx, LightComponent* light, TransformComponent* transform);

    void RenderGBuffer(Renderer::RenderContext* ctx, CameraComponent* camera,
                       TransformComponent* cameraTransform);
    void RenderShadow(Renderer::RenderContext * ctx, LightComponent* pLight, uint32 faceID = 0);
    void FindActiveMeshEntities();
    void FindVisibleMeshEntities(const Math::Frustum & frustum, std::vector<MeshEntry>& list);

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
