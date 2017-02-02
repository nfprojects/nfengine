/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system.
 */

#pragma once

#include "RendererSystem.hpp"

#include "../../Renderer/RendererResources.hpp"
#include "../../Renderers/RendererInterface/CommandRecorder.hpp"

#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Containers/PackedArray.hpp"
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

    std::unique_ptr<Renderer::ShadowMap> shadowMap;
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

    std::vector<LightProxy*> visibleOmniLights;
    std::vector<LightProxy*> visibleSpotLights;

    Common::TaskID shadowPassTask;
    Common::TaskID geometryPassTask;
    Common::TaskID lightsPassTask;
    Common::TaskID debugLayerTask;

    RenderingData();
    RenderingData(const RenderingData& other);
};

using RenderingDataPtr = std::shared_ptr<RenderingData>;


/**
 * Scene system - high level renderer (implementation).
 */
// TODO move to "nfRenderer" project
class NFE_ALIGN(16) RendererSystem final
    : public IRendererSystem
    , public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(RendererSystem)

public:
    explicit RendererSystem(SceneManager* scene);
    ~RendererSystem();

    // ISystem interface implementation
    void Update(float dt) override;


    // IRendererSystem interface implementation:

    void SetEnvironment(const EnvironmentDesc& desc) override;
    void GetEnvironment(EnvironmentDesc& desc) const override;

    bool Render(const Common::TaskContext& context, const Renderer::View* view) override;

    RenderProxyID CreateMeshProxy(const MeshProxyDesc& desc) override;
    bool UpdateMeshProxy(const RenderProxyID proxyID, const MeshProxyDesc& desc) override;
    bool DeleteMeshProxy(const RenderProxyID proxyID) override;

    RenderProxyID CreateLightProxy(const LightProxyDesc& desc) override;
    bool UpdateLightProxy(const RenderProxyID proxyID, const LightProxyDesc& desc) override;
    bool DeleteLightProxy(const RenderProxyID proxyID) override;

private:
    Common::TaskID mUpdateTask;

    EnvironmentDesc mEnvironment;

    Common::PackedArray<MeshProxy, RenderProxyID, 16> mMeshProxies;
    Common::PackedArray<LightProxy, RenderProxyID, 16> mLightProxies;

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
