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
    friend class SceneManager;
    friend class MeshComponent;
    friend class LightComponent;

    SceneManager* mScene;

    std::vector<std::tuple<TransformComponent*, MeshComponent*>> mMeshes;
    std::vector<std::tuple<TransformComponent*, MeshComponent*>> mActiveMeshEntities;
    std::vector<std::tuple<TransformComponent*, LightComponent*>> mLights;
    std::vector<std::tuple<TransformComponent*, CameraComponent*>> mCameras;

    void RenderGBuffer(Renderer::RenderContext* pCtx, CameraComponent* pCamera);
    void RenderShadow(Renderer::RenderContext * pCtx, LightComponent* pLight, uint32 faceID = 0);
    void FindActiveMeshEntities();
    void FindVisibleMeshEntities(const Math::Frustum & frustum, std::vector<MeshComponent*>* pList);

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
