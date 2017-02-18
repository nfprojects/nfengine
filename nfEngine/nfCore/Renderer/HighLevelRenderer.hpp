/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of main class of high-level Renderer
 */

#pragma once

#include "RendererResources.hpp"
#include "RendererContext.hpp"
#include "../Renderers/RendererInterface/Device.hpp"

#include "nfCommon/Library.hpp"


namespace NFE {
namespace Renderer {

struct GPUStats
{
    uint32 IAVertices;
    uint32 IAPrimitives;
    uint32 VSInvocations;
    uint32 GSInvocations;
    uint32 GSPrimitives;
    uint32 CInvocations;
    uint32 CPrimitives;
    uint32 PSInvocations;
    uint32 HSInvocations;
    uint32 DSInvocations;
    uint32 CSInvocations;

    GPUStats()
        : IAVertices(0)
        , IAPrimitives(0)
        , VSInvocations(0)
        , GSInvocations(0)
        , GSPrimitives(0)
        , CInvocations(0)
        , CPrimitives(0)
        , PSInvocations(0)
        , HSInvocations(0)
        , DSInvocations(0)
        , CSInvocations(0)
    {}
};


/**
 * Global renderer configuration.
 */
struct RendererConfig
{
    float gamma;

    RendererConfig();
};

class HighLevelRenderer final
{
    NFE_MAKE_NONCOPYABLE(HighLevelRenderer)
    NFE_MAKE_NONMOVEABLE(HighLevelRenderer)

private:
    Common::Library mLowLevelRendererLib;

    /// low-level renderer objects
    IDevice* mRenderingDevice;

    // format used form backbuffer creation
    ElementFormat mBackbufferFormat;

    // default sampler for 3D rendering
    std::unique_ptr<ISampler> mDefaultSampler;

    // default (empty) textures for G-Buffer Renderer
    std::unique_ptr<const TexturePtr&> mDefaultDiffuseTexture;
    std::unique_ptr<const TexturePtr&> mDefaultNormalTexture;
    std::unique_ptr<const TexturePtr&> mDefaultSpecularTexture;

    /// TODO: make sure there is no false sharing problem here
    std::unique_ptr<RenderContext[]> mDeferredContexts;

    std::vector<std::unique_ptr<ICommandList>> mGeometryCLs;
    std::vector<std::unique_ptr<ICommandList>> mShadowsCLs;
    std::vector<std::unique_ptr<ICommandList>> mLightsCLs;
    std::vector<std::unique_ptr<ICommandList>> mDebugCLs;

    RendererConfig mConfig;

    void CreateCommonResources();

public:
    GPUStats pipelineStats;


    HighLevelRenderer();
    ~HighLevelRenderer();

    /**
     * Initialize the renderer.
     *
     * @param preferredRendererName Low-level renderer name
     * @return 0 on success.
     */
    bool Init();
    void Release();
    void ReleaseModules();

    /**
     * Get low-level rendering device.
     */
    IDevice* GetDevice();

    /**
     * Get deferred (secondary) rendering context.
     * Using a deferred context will buffer commands only.
     * One deferred context can be used only by one thread.
     *
     * @param id Deferred context ID.
     */
    RenderContext* GetDeferredContext(size_t id) const;

    /**
     * Prepare command buffers for recording.
     */
    void ResetCommandBuffers();

    /**
     * Execute recorded command buffers.
     */
    void FinishAndExecuteCommandBuffers();

    /**
     * Get shaders location.
     */
    std::string GetShadersPath() const;

    NFE_INLINE ElementFormat GetBackbufferFormat() const
    {
        return mBackbufferFormat;
    }

    /**
     * Get default sampler for 3D meshes rendering.
     */
    NFE_INLINE ISampler* GetDefaultSampler() const
    {
        return mDefaultSampler.get();
    }

    NFE_INLINE const TexturePtr& GetDefaultDiffuseTexture() const
    {
        return mDefaultDiffuseTexture.get();
    }

    NFE_INLINE const TexturePtr& GetDefaultNormalTexture() const
    {
        return mDefaultNormalTexture.get();
    }

    NFE_INLINE const TexturePtr& GetDefaultSpecularTexture() const
    {
        return mDefaultSpecularTexture.get();
    }

    NFE_INLINE Math::Vector GammaFix(const Math::Vector& color) const
    {
        return color * color;
    }
};

} // namespace Renderer
} // namespace NFE
