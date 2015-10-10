/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of main class of high-level Renderer
 */

#pragma once

#include "RendererResources.hpp"
#include "RendererContext.hpp"

#include "../nfCommon/Library.hpp"
#include "../Renderers/RendererInterface/Device.hpp"

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

class HighLevelRenderer final
{
private:
    Common::Library mLowLevelRendererLib;

    /// low-level renderer objects
    IDevice* mRenderingDevice;
    ICommandBuffer* mCommandBuffer;

    // default blend state (for rendering without alpha blending)
    std::unique_ptr<IBlendState> mDefaultBlendState;
    // default sampler for 3D rendering
    std::unique_ptr<ISampler> mDefaultSampler;
    // default depth state for 3D rendering with depth testing enabled
    std::unique_ptr<IDepthState> mDefaultDepthState;
    // default rasterizer state (solid, no culling)
    std::unique_ptr<IRasterizerState> mDefaultRasterizerState;

    // default (empty) textures for G-Buffer Renderer
    std::unique_ptr<ITexture> mDefaultDiffuseTexture;
    std::unique_ptr<ITexture> mDefaultNormalTexture;
    std::unique_ptr<ITexture> mDefaultSpecularTexture;

    /// TODO: make sure there is no false sharing problem here
    std::unique_ptr<RenderContext[]> mDeferredContexts;
    std::unique_ptr<RenderContext> mImmediateContext;

    /// disable unwanted methods
    HighLevelRenderer(const HighLevelRenderer&) = delete;
    HighLevelRenderer(HighLevelRenderer&&) = delete;
    HighLevelRenderer& operator=(const HighLevelRenderer&) = delete;
    HighLevelRenderer& operator=(HighLevelRenderer&&) = delete;

    void CreateCommonResources();

public:
    GPUStats pipelineStats;
    RendererSettings settings;

    HighLevelRenderer();
    ~HighLevelRenderer();

    /**
     * Initialize the renderer.
     *
     * @param preferredRendererName Low-level renderer name
     * @return 0 on success.
     */
    bool Init(const std::string& preferredRendererName);
    void Release();

    /**
     * Get low-level rendering device.
     */
    IDevice* GetDevice();

    /**
     * Get immediate (primary) rendering context.
     * Using the immediate context will result in direct rendering.
     * It can be only used by the main thread.
     */
    RenderContext* GetImmediateContext() const;

    /**
     * Get deferred (secondary) rendering context.
     * Using a deferred context will buffer commands only.
     * One deferred context can be used only by one thread.
     *
     * @param id Deferred context ID.
     */
    RenderContext* GetDeferredContext(size_t id) const;

    /**
     * Get shaders location.
     */
    std::string GetShadersPath() const;


    /**
     * Get default blend state for rendering without alpha blending.
     */
    NFE_INLINE IBlendState* GetDefaultBlendState() const
    {
        return mDefaultBlendState.get();
    }

    /**
     * Get default sampler for 3D meshes rendering.
     */
    NFE_INLINE ISampler* GetDefaultSampler() const
    {
        return mDefaultSampler.get();
    }

    /**
     * Get default depth state for 3D rendering with depth testing enabled.
     */
    NFE_INLINE IDepthState* GetDefaultDepthState() const
    {
        return mDefaultDepthState.get();
    }

    /**
     * Get default rasterizer state (solid, no culling).
     */
    NFE_INLINE IRasterizerState* GetDefaultRasterizerState() const
    {
        return mDefaultRasterizerState.get();
    }


    NFE_INLINE ITexture* GetDefaultDiffuseTexture() const
    {
        return mDefaultDiffuseTexture.get();
    }

    NFE_INLINE ITexture* GetDefaultNormalTexture() const
    {
        return mDefaultNormalTexture.get();
    }

    NFE_INLINE ITexture* GetDefaultSpecularTexture() const
    {
        return mDefaultSpecularTexture.get();
    }
};

} // namespace Renderer
} // namespace NFE
