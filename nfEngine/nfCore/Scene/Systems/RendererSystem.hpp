/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system.
 */

#pragma once

#include "../../Core.hpp"
#include "../../Renderer/View.hpp"
#include "System.hpp"
#include "RenderProxies.hpp"

#include "nfCommon/Utils/ThreadPool.hpp"
#include "nfCommon/Memory/Aligned.hpp"


namespace NFE {
namespace Scene {


/**
 * Rendering scene environment properties.
 */
class CORE_API NFE_ALIGN16 EnvironmentDesc
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
 * Scene system - high level renderer (interface).
 */
class IRendererSystem : public ISystem
{
    NFE_MAKE_NONCOPYABLE(IRendererSystem)

public:
    IRendererSystem(SceneManager* scene)
        : ISystem(scene)
    { }

    /**
     * Set scene environment description.
     */
    virtual void SetEnvironment(const EnvironmentDesc& desc) = 0;

    /**
     * Get scene environment description.
     */
    virtual void GetEnvironment(EnvironmentDesc& desc) const = 0;

    /**
     * Render the scene for a view.
     * This function will be called as thread pool task.
     *
     * @param context   Thread pool's task context.
     * @param view      Target viewport.
     */
    virtual bool Render(const Common::TaskContext& context, const Renderer::View* view) = 0;

    /**
     * Create a new mesh rendering proxy.
     * @param   desc    Rendering proxy descriptor.
     * @return  Rendering proxy ID.
     */
    virtual RenderProxyID CreateMeshProxy(const MeshProxyDesc& desc) = 0;

    /**
     * Create a new light rendering proxy.
     * @param   desc    Rendering proxy descriptor.
     * @return  Rendering proxy ID.
     */
    virtual RenderProxyID CreateLightProxy(const LightProxyDesc& desc) = 0;

    /**
     * Update mesh rendering proxy. For example change mesh or it's transformation matrix.
     * @param   desc    New rendering proxy descriptor.
     * @return  True on success.
     */
    virtual bool UpdateMeshProxy(const RenderProxyID proxyID, const MeshProxyDesc& desc) = 0;

    /**
     * Update light rendering proxy. For example change mesh or it's transformation matrix.
     * @param   desc    New rendering proxy descriptor.
     * @return  True on success.
     */
    virtual bool UpdateLightProxy(const RenderProxyID proxyID, const LightProxyDesc& desc) = 0;

    /**
     * Delete mesh rendering proxy.
     * @param   proxyID Valid proxy ID.
     * @return  True on success.
     */
    virtual bool DeleteMeshProxy(const RenderProxyID proxyID) = 0;

    /**
     * Delete light rendering proxy.
     * @param   proxyID Valid proxy ID.
     * @return  True on success.
     */
    virtual bool DeleteLightProxy(const RenderProxyID proxyID) = 0;
};

} // namespace Scene
} // namespace NFE
