/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  All engine's global variables definitions.
 */

#include "PCH.hpp"
#include "Globals.hpp"
#include "Renderer/Renderer.hpp"

namespace NFE {

using namespace Renderer;
using namespace Resource;

const std::string g_DataPath = "nfEngineTest/Data/";
const std::string g_ShadersPath = "nfEngine/nfRenderer/Shaders/";

std::unique_ptr<HighLevelRenderer> g_pRenderer;
PostProcessRenderer* g_pPostProcessRenderer = NULL;
GuiRenderer* g_pGuiRenderer = NULL;
LightsRenderer* g_pLightRenderer = NULL;
DebugRenderer* g_pDebugRenderer = NULL;
GBufferRenderer* g_pGBufferRenderer = NULL;
ShadowRenderer* g_pShadowRenderer = NULL;

size_t g_DeferredContextsNum = 0;
RenderContext* g_pImmediateContext = NULL;
RenderContext** g_pDeferredContexts = NULL;

std::shared_ptr<Common::ThreadPool> g_pMainThreadPool;
ResManager* g_pResManager = NULL;

} // namespace NFE
