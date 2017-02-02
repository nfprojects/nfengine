/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system.
 */

#pragma once

#include "../../Core.hpp"

#include "System.hpp"
#include "RenderProxies.hpp"

// TODO remove
#include "../../Renderer/View.hpp"

#include "../../Renderer/RendererResources.hpp"
#include "../../Renderers/RendererInterface/CommandRecorder.hpp"

#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Containers/PackedArray.hpp"
#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Math/Frustum.hpp"


// TODO
// the whole Renderer System must be moved to separate project
// and this header must be split into multiple, because it's getting too big.
// But it's task for another commit

namespace NFE {
namespace Scene {

/**
 * Mesh Proxy - descriptor plus additional internal data
 */
struct NFE_ALIGN16 MeshProxy
{
    MeshProxyDesc desc;

    Math::Box globalBox;    // global AABB
};

/**
 * Light Proxy - descriptor plus additional internal data
 */
struct NFE_ALIGN16 LightProxy
{
    LightProxyDesc desc;

    Math::Frustum frustum;      // global frustum
    Math::Matrix viewMatrix;
    Math::Matrix projMatrix;

    std::unique_ptr<Renderer::ShadowMap> shadowMap;
    Renderer::ResourceBindingInstancePtr lightMapBindingInstance;

    // was shadow map already rendered?
    // this is required in case of multiple views rendering the same scene
    std::atomic<bool> shadowDrawn;

    NFE_INLINE LightProxy()
        : shadowDrawn(false)
    {
    }

    NFE_INLINE LightProxy(LightProxy&& other)
    {
        desc = std::move(other.desc);
        shadowMap = std::move(other.shadowMap);
        lightMapBindingInstance = std::move(other.lightMapBindingInstance);

        frustum = other.frustum;
        viewMatrix = other.viewMatrix;
        projMatrix = other.projMatrix;
        shadowDrawn = false;
    }
};

/**
 * Temporary data used during single View rendering.
 */
 // TODO: there are "false sharing" problems here
struct NFE_ALIGN16 RenderingData
{
    const Renderer::View* view; // TODO remove
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

struct GeometryRenderingContext
{
    Math::Vector cameraPosition;
    Math::Frustum frustum;  // camera frustum
};

/**
 * Scene system - high level renderer.
 */
class RendererSystem final : public ISystem
{
    NFE_MAKE_NONCOPYABLE(RendererSystem)

public:
    RendererSystem(SceneManager* scene);
    ~RendererSystem();

    /**
     * Update the renderer system.
     * @param dt       Time delta in seconds.
     */
    void Update(float dt) override;

    /**
     * Render the scene for a view.
     * This function will be called as thread pool task.
     *
     * @param context       Thread pool's task context.
     * @param renderingData Temporary data used for the view rendering.
     */
    void Render(const Common::TaskContext& context, RenderingData& renderingData);

    /**
     * Create a new mesh rendering proxy.
     * @param   desc    Rendering proxy descriptor.
     * @return  Rendering proxy ID.
     */
    RenderProxyID CreateMeshProxy(const MeshProxyDesc& desc);

    /**
     * Create a new light rendering proxy.
     * @param   desc    Rendering proxy descriptor.
     * @return  Rendering proxy ID.
     */
    RenderProxyID CreateLightProxy(const LightProxyDesc& desc);

    /**
     * Update mesh rendering proxy. For example change mesh or it's transformation matrix.
     * @param   desc    New rendering proxy descriptor.
     * @return  True on success.
     */
    bool UpdateMeshProxy(const RenderProxyID proxyID, const MeshProxyDesc& desc);

    /**
     * Update light rendering proxy. For example change mesh or it's transformation matrix.
     * @param   desc    New rendering proxy descriptor.
     * @return  True on success.
     */
    bool UpdateLightProxy(const RenderProxyID proxyID, const LightProxyDesc& desc);

    /**
     * Delete mesh rendering proxy.
     * @param   proxyID Valid proxy ID.
     * @return  True on success.
     */
    bool DeleteMeshProxy(const RenderProxyID proxyID);

    /**
     * Delete light rendering proxy.
     * @param   proxyID Valid proxy ID.
     * @return  True on success.
     */
    bool DeleteLightProxy(const RenderProxyID proxyID);

private:
    Common::TaskID mUpdateTask;

    Common::PackedArray<MeshProxy, RenderProxyID> mMeshProxies;
    Common::PackedArray<LightProxy, RenderProxyID> mLightProxies;

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
     * @param cameraPosition  Used for distance calculation.
     */
    void RenderGeometry(Renderer::GeometryRendererContext* ctx,
                        const Math::Frustum& viewFrustum, const Math::Vector& cameraPosition) const;

    // Find visible mesh proxies in a given frustum
    void FindVisibleMeshes(const Math::Frustum& frustum, std::vector<MeshProxy*>& out) const;

    // Find visible light proxies in a given frustum
    // Result will be stored in RenderingData structure
    void FindVisibleLights(const Math::Frustum & frustum, RenderingData& data) const;

    void UpdateLights();
};

} // namespace Scene
} // namespace NFE
