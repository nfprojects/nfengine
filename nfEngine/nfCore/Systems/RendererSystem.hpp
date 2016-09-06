/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system.
 */

#pragma once

#include "../Core.hpp"

#include "System.hpp"

#include "../Renderer/RendererResources.hpp"
#include "../Renderers/RendererInterface/CommandBuffer.hpp"

#include "nfCommon/Aligned.hpp"
#include "nfCommon/PackedArray.hpp"
#include "nfCommon/ThreadPool.hpp"
#include "nfCommon/Math/Frustum.hpp"

namespace NFE {
namespace Scene {

struct NFE_ALIGN16 MeshProxy
{
    Math::Matrix transform;
    Math::Vector velocity;
    Math::Vector angularVelocity;
    Mesh* mesh;
};

struct NFE_ALIGN16 LightProxy
{
    Math::Matrix transform;
    Math::Frustum frustum;
    Math::Matrix viewMatrix;
    Math::Matrix projMatrix;

    LightComponent* light;

    std::atomic_bool shadowDrawn;

    // required because of atomic member
    NFE_INLINE LightProxy& operator= (const LightProxy& other)
    {
        frustum = other.frustum;
        viewMatrix = other.viewMatrix;
        projMatrix = other.projMatrix;
        transform = other.transform;
        light = other.light;
        shadowDrawn = false;
        return *this;
    }
};

/**
 * Temporary data used during single View rendering.
 */
 // TODO: there are "false sharing" problems here
struct NFE_ALIGN16 RenderingData
{
    typedef std::unique_ptr<Renderer::ICommandList> CommandListPtr;

    Renderer::View* view;
    Renderer::CameraRenderDesc cameraRenderDesc;

    std::vector<LightProxy*> visibleOmniLights;
    std::vector<LightProxy*> visibleSpotLights;

    Common::TaskID sceneRenderTask;

    Common::TaskID shadowPassTask;
    Common::TaskID geometryPassTask;
    Common::TaskID lightsPassTask;
    Common::TaskID debugLayerTask;

    RenderingData();
    RenderingData(const RenderingData& other);
    void WaitForRenderingTasks() const;
};

NFE_ALIGN16
class RendererSystem
    : public ISystem
    , public Common::Aligned<16>
{
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

    /**
     * Create rendering mesh proxy.
     */
    uint32 CreateMeshProxy(const MeshProxy& data);

    /**
     * Create rendering light proxy.
     */
    uint32 CreateLightProxy(const LightProxy& data);

    // TODO: updating proxies
    // TODO: removing proxies

private:
    Common::TaskID mUpdateTask;

    Common::PackedArray<MeshProxy> mMeshProxies;
    Common::PackedArray<LightProxy> mLightProxies;

    // which spot lights have its shadow map already rendered?
    std::unique_ptr<std::atomic_bool[]> mSpotLightsShadowDrawn;
    // which omni lights have its shadow map already rendered?
    std::unique_ptr<std::atomic_bool[]> mOmniLightsShadowDrawn;

    // rendering tasks
    void RenderDebugLayer(const Common::TaskContext& context, RenderingData& data) const;
    void RenderLightsDebug(RenderingData& data, Renderer::RenderContext* ctx) const;
    void RenderShadowMaps(const Common::TaskContext& context, RenderingData& data);
    void RenderSpotShadowMap(const Common::TaskContext& context, const LightProxy* lightData, RenderingData& data) const;
    void RenderOmniShadowMap(const Common::TaskContext& context, const LightProxy* lightData, RenderingData& data) const;
    void RenderLights(const Common::TaskContext& context, RenderingData& data) const;

    /**
     * Draw all the geometry visible in the @p viewFrustum using GeometryRenderer.
     * @param ctx             Renderer context
     * @param viewFrustum     A frustum against which frustum culling will be performed
     * @param cameraTransform Transform component of the viewing camera.
     *                        Used for distance calculation.
     */
    void RenderGeometry(Renderer::GeometryRendererContext* ctx, const Math::Frustum& viewFrustum) const;

    void FindVisibleMeshEntities(const Math::Frustum & frustum, std::vector<MeshProxy>& list) const;
    void FindVisibleLights(const Math::Frustum & frustum, RenderingData& data);

    void UpdateCameras();
    void UpdateLights();
};

} // namespace Scene
} // namespace NFE
