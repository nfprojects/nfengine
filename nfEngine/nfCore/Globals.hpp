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
extern Render::IRenderer* g_pRenderer;
extern Render::IPostProcessRenderer* g_pPostProcessRenderer;
extern Render::IGuiRenderer* g_pGuiRenderer;
extern Render::ILightsRenderer* g_pLightRenderer;
extern Render::IDebugRenderer* g_pDebugRenderer;
extern Render::IGBufferRenderer* g_pGBufferRenderer;
extern Render::IShadowRenderer* g_pShadowRenderer;


/// Immediate (main) and deferred (secondary) rendering contexts
extern size_t g_DeferredContextsNum;
extern Render::IRenderContext* g_pImmediateContext;
extern Render::IRenderContext** g_pDeferredContexts;

extern std::shared_ptr<Common::ThreadPool> g_pMainThreadPool;
extern Resource::ResManager* g_pResManager;

} // namespace NFE
