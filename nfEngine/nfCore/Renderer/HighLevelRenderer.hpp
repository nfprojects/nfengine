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

class HighLevelRenderer
{
private:
    std::unique_ptr<RenderContext> mImmediateContext;

public:
    GPUStats pipelineStats;
    RendererSettings settings;

    void* GetDevice() const;
    int Init();
    void Begin();
    void SwapBuffers(IRenderTarget* pRenderTarget, ViewSettings* pViewSettings, float dt);
    void ExecuteDeferredContext(RenderContext* pContext);

    RenderContext* GetImmediateContext() const;
    RenderContext* CreateDeferredContext();
};

} // namespace Renderer
} // namespace NFE
