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

    /// TODO: make sure there is no false sharing problem here
    std::unique_ptr<RenderContext[]> mDeferredContexts;
    std::unique_ptr<RenderContext> mImmediateContext;

    /// disable unwanted methods
    HighLevelRenderer(const HighLevelRenderer&) = delete;
    HighLevelRenderer(HighLevelRenderer&&) = delete;
    HighLevelRenderer& operator=(const HighLevelRenderer&) = delete;
    HighLevelRenderer& operator=(HighLevelRenderer&&) = delete;

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
     * Update renderer view and display it on a screen if needed.
     */
    void ProcessView(View* view);

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
     * Execute buffered by deferred context commands on the immediate context.
     *
     * @param context A deferred context.
     */
    void ExecuteDeferredContext(RenderContext* context);

    /**
     * Get shaders location.
     */
    std::string GetShadersPath() const;
};

} // namespace Renderer
} // namespace NFE
