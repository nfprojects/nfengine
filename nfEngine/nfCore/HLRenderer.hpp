/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of main class of high-level Renderer
 */

#pragma once

#include "Renderer.hpp"
#include "RendererResources.hpp"
#include "RendererContext.hpp"

#include "RendererContext.hpp"
#include "GuiRenderer.hpp"
#include "PostProcessRenderer.hpp"
#include "DebugRenderer.hpp"
#include "ShadowsRenderer.hpp"
#include "GeometryBufferRenderer.hpp"
#include "LightsRenderer.hpp"

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

struct GPUFeatures
{
    // TODO
};

// TODO: remove inheritance
class HighLevelRenderer
{
private:
    std::unique_ptr<RenderContext> mImmediateContext;
    std::unique_ptr<GuiRenderer> mGuiRenderer;
    std::unique_ptr<PostProcessRenderer> mPostProcessRenderer;
    std::unique_ptr<DebugRenderer> mDebugRenderer;
    std::unique_ptr<ShadowRenderer> mShadowRenderer;
    std::unique_ptr<GBufferRenderer> mGBufferRenderer;
    std::unique_ptr<LightsRenderer> mLightsRenderer;

public:
    GPUStats pipelineStats;
    GPUFeatures gpuFeatures;
    RendererSettings settings;

    void* GetDevice() const;
    int Init();
    int InitModules();
    void ReleaseModules();

    void Begin();
    void SwapBuffers(IRenderTarget* pRenderTarget, ViewSettings* pViewSettings, float dt);

    void ExecuteDeferredContext(RenderContext* pContext);

    GuiRenderer* GetGuiRenderer() const;
    PostProcessRenderer* GetPostProcessRenderer() const;
    DebugRenderer* GetDebugRenderer() const;
    ShadowRenderer* GetShadowRenderer() const;
    GBufferRenderer* GetGBufferRenderer() const;
    LightsRenderer* GetLightsRenderer() const;

    RenderContext* GetImmediateContext() const;
    RenderContext* CreateDeferredContext();
};

} // namespace Renderer
} // namespace NFE
