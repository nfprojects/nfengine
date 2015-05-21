/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  All engine's global variables declarations.
 */

#pragma once

#include "Core.hpp"
#include "Prerequisites.hpp"

namespace NFE {

/**
 *  Resources paths, relative to executable.
 *  Temporary - This must be changed!!!
 */
extern const std::string g_DataPath;
extern const std::string g_ShadersPath;

/// Renderer modules
extern std::unique_ptr<Renderer::HighLevelRenderer> g_pRenderer;
extern Renderer::PostProcessRenderer* g_pPostProcessRenderer;
extern Renderer::GuiRenderer* g_pGuiRenderer;
extern Renderer::LightsRenderer* g_pLightRenderer;
extern Renderer::DebugRenderer* g_pDebugRenderer;
extern Renderer::GBufferRenderer* g_pGBufferRenderer;
extern Renderer::ShadowRenderer* g_pShadowRenderer;


/// Immediate (main) and deferred (secondary) rendering contexts
extern size_t g_DeferredContextsNum;
extern Renderer::RenderContext* g_pImmediateContext;
extern Renderer::RenderContext** g_pDeferredContexts;

extern std::shared_ptr<Common::ThreadPool> g_pMainThreadPool;
extern Resource::ResManager* g_pResManager;

} // namespace NFE
