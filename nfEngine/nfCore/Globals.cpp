/**
    NFEngine project

    \file   Globals.cpp
    \brief  All engine's global variables definitions.
*/

#include "stdafx.hpp"
#include "Globals.hpp"

namespace NFE {

using namespace Render;
using namespace Resource;

const std::string g_DataPath = "nfEngineTest/Data/";
const std::string g_ShadersPath = "nfEngine/nfRendererD3D11/Shaders/";

IRenderer* g_pRenderer = NULL;
IPostProcessRenderer* g_pPostProcessRenderer = NULL;
IGuiRenderer* g_pGuiRenderer = NULL;
ILightsRenderer* g_pLightRenderer = NULL;
IDebugRenderer* g_pDebugRenderer = NULL;
IGBufferRenderer* g_pGBufferRenderer = NULL;
IShadowRenderer* g_pShadowRenderer = NULL;

uint32 g_DeferredContextsNum = 0;
IRenderContext* g_pImmediateContext = NULL;
IRenderContext** g_pDeferredContexts = NULL;

std::shared_ptr<Common::ThreadPool> g_pMainThreadPool;
ResManager* g_pResManager = NULL;

} // namespace NFE
