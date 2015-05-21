/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of main class of high-level Renderer
 */

#pragma once

#include "RendererResources.hpp"
#include "RendererContext.hpp"

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
    std::unique_ptr<RenderContext> mImmediateContext;

public:
    GPUStats pipelineStats;
    RendererSettings settings;

    /**
     * Get rendering API handle.
     */
    void* GetDevice() const;

    /**
     * Initialize the renderer.
     *
     * @return 0 on success.
     */
    int Init();

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
};

} // namespace Renderer
} // namespace NFE
