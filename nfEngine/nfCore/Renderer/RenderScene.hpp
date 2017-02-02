/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system.
 */

#pragma once

#include "../Core.hpp"
#include "RenderProxies.hpp"
#include "RendererResources.hpp"

#include "../../Renderers/RendererInterface/CommandRecorder.hpp"

#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Containers/PackedArray.hpp"
#include "nfCommon/Math/Frustum.hpp"
#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Containers/UniquePtr.hpp"

// TODO
// the whole Renderer System must be moved to separate project
// and this header must be split into multiple, because it's getting too big.
// But it's task for another commit

namespace NFE {
namespace Renderer {


/**
 * Rendering scene environment properties.
 */
class CORE_API NFE_ALIGN(16) EnvironmentDesc
    : public Common::Aligned<16>
{
public:
    Math::Vector ambientLight;
    Math::Vector backgroundColor;

    EnvironmentDesc()
    {
        ambientLight = Math::Vector(0.3f, 0.3f, 0.3f);
        backgroundColor = Math::Vector(0.3f, 0.3f, 0.3f);
    }
};

/**
 * Mesh Proxy - descriptor plus additional internal data
 */
struct NFE_ALIGN(16) MeshProxy
{
    MeshProxyDesc desc;

    Math::Box globalBox;    // global AABB
};


/**
 * Light Proxy - descriptor plus additional internal data
 */
struct NFE_ALIGN(16) LightProxy
{
    LightProxyDesc desc;

    Math::Frustum frustum;      // global-space frustum
    Math::Matrix viewMatrix;
    Math::Matrix projMatrix;

    Common::UniquePtr<Renderer::ShadowMap> shadowMap;
    Renderer::ResourceBindingInstancePtr lightMapBindingInstance;

    // was shadow map already rendered?
    // this is required in case of multiple views rendering the same scene
    std::atomic<bool> shadowDrawn;

    LightProxy()
        : shadowDrawn(false)
    {
    }

    LightProxy(LightProxy&& other)
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


struct GeometryRenderingContext
{
    Math::Vector cameraPosition;
    Math::Frustum frustum;  // camera frustum
};


/**
 * Temporary data used during single View rendering.
 */
// TODO there are "false sharing" problems here
struct NFE_ALIGN(16) RenderingData
    : public Common::Aligned<16>
{
    const Renderer::View* view; // TODO remove
    Renderer::CameraRenderDesc cameraRenderDesc;

    Common::DynArray<LightProxy*> visibleOmniLights;
    Common::DynArray<LightProxy*> visibleSpotLights;

    Common::TaskID shadowPassTask;
    Common::TaskID geometryPassTask;
    Common::TaskID lightsPassTask;
    Common::TaskID debugLayerTask;

    RenderingData();
    RenderingData(const RenderingData& other);
};

using RenderingDataPtr = Common::SharedPtr<RenderingData>;


/**
 * Rendering scene.
 * Allows for placing rendering proxies (meshes, lights, etc.) and render it to a texture.
 */
// TODO move to "nfRenderer" project
// TODO hide implementation (IRenderScene)
class CORE_API NFE_ALIGN(16) RenderScene final
    : public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(RenderScene)

public:
    RenderScene();
    ~RenderScene();

    void Update(float dt);

    /**
     * Set scene environment description.
     */
    void SetEnvironment(const EnvironmentDesc& desc);

    /**
     * Get scene environment description.
     */
    void GetEnvironment(EnvironmentDesc& desc) const;

    /**
     * Render the scene for a view.
     * This function will be called as thread pool task.
     *
     * @param context   Thread pool's task context.
     * @param view      Target viewport.
     */
    bool Render(const Common::TaskContext& context, const Renderer::View* view);

    /**
     * Create a new mesh rendering proxy.
     * @param   desc    Rendering proxy descriptor.
     * @return  Rendering proxy ID.
     */
    ProxyID CreateMeshProxy(const MeshProxyDesc& desc);

    /**
     * Update mesh rendering proxy. For example change mesh or it's transformation matrix.
     * @param   desc    New rendering proxy descriptor.
     * @return  True on success.
     */
    bool UpdateMeshProxy(const ProxyID proxyID, const MeshProxyDesc& desc);

    /**
     * Delete mesh rendering proxy.
     * @param   proxyID Valid proxy ID.
     */
    void DeleteMeshProxy(const ProxyID proxyID);

    /**
     * Create a new light rendering proxy.
     * @param   desc    Rendering proxy descriptor.
     */
    ProxyID CreateLightProxy(const LightProxyDesc& desc);

    /**
     * Update light rendering proxy. For example change mesh or it's transformation matrix.
     * @param   desc    New rendering proxy descriptor.
     * @return  True on success.
     */
    bool UpdateLightProxy(const ProxyID proxyID, const LightProxyDesc& desc);

    /**
     * Delete light rendering proxy.
     * @param   proxyID Valid proxy ID.
     */
    void DeleteLightProxy(const ProxyID proxyID);

private:
    Common::TaskID mUpdateTask;

    EnvironmentDesc mEnvironment;

    Common::PackedArray<MeshProxy, ProxyID, 16> mMeshProxies;
    Common::PackedArray<LightProxy, ProxyID, 16> mLightProxies;

    // which spot lights have its shadow map already rendered?
    Common::UniquePtr<std::atomic_bool[]> mSpotLightsShadowDrawn;
    // which omni lights have its shadow map already rendered?
    Common::UniquePtr<std::atomic_bool[]> mOmniLightsShadowDrawn;

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
    void FindVisibleMeshes(const Math::Frustum& frustum, Common::DynArray<MeshProxy*>& out) const;

    // Find visible light proxies in a given frustum
    // Result will be stored in RenderingData structure
    void FindVisibleLights(const Math::Frustum & frustum, RenderingData& data) const;

    void UpdateLights();
};

} // namespace Renderer
} // namespace NFE
