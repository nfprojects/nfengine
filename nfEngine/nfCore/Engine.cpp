/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic engine's API implementation.
 */

#include "PCH.hpp"
#include "Globals.hpp"
#include "Engine.hpp"
#include "Texture.hpp"
#include "Performance.hpp"
#include "Entity.hpp"

#include "Renderer/HighLevelRenderer.hpp"
#include "Renderer/GuiRenderer.hpp"
#include "Renderer/View.hpp"

#include "../nfCommon/Memory.hpp"
#include "../nfCommon/Window.hpp"
#include "../nfCommon/Timer.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/SystemInfo.hpp"

namespace NFE {

using namespace Renderer;
using namespace Scene;
using namespace Resource;

std::set<SceneManager*> g_Scenes;
bool g_Initialized = 0;

Common::Timer g_Timer;


SceneManager* EngineCreateScene()
{
    SceneManager* pScene = new SceneManager;
    g_Scenes.insert(pScene);
    return pScene;
}


Result EngineDeleteScene(SceneManager* pScene)
{
    if (g_Scenes.erase(pScene))
    {
        delete pScene;
        return Result::OK;
    }

    return Result::AlreadyFree;
}

Result InitAntTweakBar()
{
    int err = TwInit(TW_DIRECT3D11, gRenderer->GetDevice()->GetHandle());
    if (err != 1)
    {
        LOG_ERROR("Failed to initialize AntTweakBar.");
        return Result::Error;
    }

    // TODO: restore after high-level renderer is useable
    /*
    TwBar* pDebugBar = TwNewBar("Debugging");
    TwDefine("Debugging iconified=true color='50 50 50' alpha=200 refresh=0.25");
    TwAddVarRW(pDebugBar, "Debug enable", TW_TYPE_BOOLCPP, &g_pRenderer->settings.debugEnable,
               "help = 'Enable debug rendering.'");
    TwAddVarRW(pDebugBar, "Debug meshes", TW_TYPE_BOOLCPP, &g_pRenderer->settings.debugMeshes,
               "help = 'Draw AABBs for meshes.'");
    TwAddVarRW(pDebugBar, "Debug lights", TW_TYPE_BOOLCPP, &g_pRenderer->settings.debugLights,
               "help = 'Draw bounding volumes for lights.'");

    TwBar* pPerfBar = TwNewBar("GPU pipeline statistics");
    TwDefine("'GPU pipeline statistics' iconified=true color='50 50 50' alpha=200 refresh=0.25");
        TwAddVarRW(pPerfBar, "Enable", TW_TYPE_BOOLCPP, &pRenderer->settings.pipelineStats, "help = 'Enable querying for GPU pipeline statistics. It can produce slight performance drop.'");
        TwAddSeparator(pPerfBar, "Geometry", 0);
        TwAddVarRO(pPerfBar, "IA Vertices", TW_TYPE_UINT32, &pRenderer->pipelineStats.IAVertices, "help = 'Number of vertices read by input assembler.'");
        TwAddVarRO(pPerfBar, "IA Primitives", TW_TYPE_UINT32, &pRenderer->pipelineStats.IAPrimitives, "help = 'Number of primitives read by input assembler.'");
        TwAddVarRO(pPerfBar, "GS Primitives", TW_TYPE_UINT32, &pRenderer->pipelineStats.GSPrimitives, "help = 'Number of primitives generated by a geometry shader.'");
        TwAddSeparator(pPerfBar, "Shaders invocations", 0);
        TwAddVarRO(pPerfBar, "VS invocations", TW_TYPE_UINT32, &pRenderer->pipelineStats.VSInvocations, "help = 'Number of times a vertex shader was been called.'");
        TwAddVarRO(pPerfBar, "GS invocations", TW_TYPE_UINT32, &pRenderer->pipelineStats.GSInvocations, "help = 'Number of times a geometry shader was been called.'");
        TwAddVarRO(pPerfBar, "HS invocations", TW_TYPE_UINT32, &pRenderer->pipelineStats.HSInvocations, "help = 'Number of times a hull shader was been called.'");
        TwAddVarRO(pPerfBar, "DS invocations", TW_TYPE_UINT32, &pRenderer->pipelineStats.DSInvocations, "help = 'Number of times a domain shader was been called.'");
        TwAddVarRO(pPerfBar, "PS invocations", TW_TYPE_UINT32, &pRenderer->pipelineStats.PSInvocations, "help = 'Number of times a pixel shader was been called.'");
        TwAddVarRO(pPerfBar, "CS invocations", TW_TYPE_UINT32, &pRenderer->pipelineStats.CSInvocations, "help = 'Number of times a compute shader was been called.'");
        TwAddSeparator(pPerfBar, "Rasterizer", 0);
        TwAddVarRO(pPerfBar, "Rasterizer invocations", TW_TYPE_UINT32, &pRenderer->pipelineStats.CInvocations, "help = 'Number of primitives that were sent to the rasterizer.'");
        TwAddVarRO(pPerfBar, "Rendered primitives", TW_TYPE_UINT32, &pRenderer->pipelineStats.CPrimitives, "help = 'Number of primitives that were rendered.'");

    TwBar* pAABar = TwNewBar("FXAA");
    TwDefine("FXAA iconified=true color='50 50 50' alpha=200 refresh=0.25");
    TwAddVarRW(pAABar, "FXAA Luma-opt", TW_TYPE_BOOLCPP, &g_pRenderer->settings.antialiasingLumaOpt,
               "help='Luma optimization enabled'");
    TwAddVarRW(pAABar, "FXAA Span-max", TW_TYPE_FLOAT, &g_pRenderer->settings.antialiasingSpanMax,
               "min=0 max=128 step=1 help='default = 8'");
    TwAddVarRW(pAABar, "FXAA Reduce-mul", TW_TYPE_FLOAT,
               &g_pRenderer->settings.antialiasingReduceMul, "min=0 max=128 step=1 help='default = 16'");
    TwAddVarRW(pAABar, "FXAA Reduce-min", TW_TYPE_FLOAT,
               &g_pRenderer->settings.antialiasingReduceMin, "min=0 max=256 step=1 help='default = 128'");

    TwBar* pBar = TwNewBar("Rendering");
    TwDefine("Rendering iconified=true color='50 50 50' alpha=200 refresh=0.25");
    TwAddVarRW(pBar, "VSync", TW_TYPE_BOOLCPP, &g_pRenderer->settings.VSync, 0);
    TwAddSeparator(pBar, "Lighting & shading", 0);
    TwAddVarRW(pBar, "Tile-Based Deferred Shading", TW_TYPE_BOOLCPP,
               &g_pRenderer->settings.tileBasedDeferredShading,
               "help = 'This feature is only available on hardware that supports Shader Model 5.0 and Compute Shaders. It can improve performance of drawing many lights on the scene by culling them agains screen tiles on GPU.'");
    TwAddSeparator(pBar, "Post process", 0);
    TwAddVarRW(pBar, "Anti-aliasing", TW_TYPE_BOOLCPP, &g_pRenderer->settings.antialiasing,
               "help = 'Enable FXAA.'");
    TwAddVarRW(pBar, "Auto exposure", TW_TYPE_BOOLCPP, &g_pRenderer->settings.autoExposure,
               "help = 'Enable automatic exposure adaptation depending on overall scene brightness.'");

    TwAddSeparator(pBar, "Bloom", 0);
    TwAddVarRW(pBar, "Bloom enable", TW_TYPE_BOOLCPP, &g_pRenderer->settings.bloom, 0);
    TwAddVarRW(pBar, "Bloom kernel size", TW_TYPE_INT32, &g_pRenderer->settings.bloomSize,
               "min=1 max=256 help='Size of blurring filter.'");
    TwAddVarRW(pBar, "Bloom variance", TW_TYPE_FLOAT, &g_pRenderer->settings.bloomVariance,
               "min=0.1 max=20 help='Variance of blurring filter.'");
    TwAddVarRW(pBar, "Bloom factor", TW_TYPE_FLOAT, &g_pRenderer->settings.bloomFactor,
               "min=0.0 max=5.0 step=0.1 help='Mixing factor.'");

    TwAddSeparator(pBar, "Motion blur", 0);
    TwAddVarRW(pBar, "Motion blur enable", TW_TYPE_BOOLCPP, &g_pRenderer->settings.motionBlur, 0);
    TwAddVarRW(pBar, "Motion blur factor", TW_TYPE_FLOAT, &g_pRenderer->settings.motionBlurFactor,
               "min=0 max=3 step = 0.1");

    TwAddSeparator(pBar, "Color", 0);
    TwAddVarRW(pBar, "Gamma correction", TW_TYPE_BOOLCPP, &g_pRenderer->settings.gammaCorrection,
               "help='Enable gamma correct rendering.'");
    TwAddVarRW(pBar, "Saturation", TW_TYPE_FLOAT, &g_pRenderer->settings.saturation,
               "min=0.0 max=2.0 step = 0.1 help='Saturation of the final, postprocessed image.'");
    TwAddVarRW(pBar, "Noise factor", TW_TYPE_FLOAT, &g_pRenderer->settings.noiseLevel,
               "min=0.0 max=2.5 step=0.01 help='Noise dithering level of the final image.'");
    TwAddVarRW(pBar, "Noise enabled", TW_TYPE_BOOLCPP, &g_pRenderer->settings.noiseEnabled,
               "help='Noise filter enabled.'");
    */

    TwDefine("GLOBAL contained=true"); // bars cannot move outside of the window

    LOG_SUCCESS("AntTweakBar initialized.");
    return Result::OK;
}

Result EngineInit()
{
    if (g_Initialized)
    {
        LOG_WARNING("Engine is already initialized!");
        return Result::AlreadyInit;
    }

    // TODO: Use SystemInfo class to check if hardware is good enough or to monitor memory usage

    /*
    PACK_RESULT pr;
    Packer_CreateReader(&g_pVfsReader);
    pr = g_pVfsReader->Init("..\\data\\Data.pak");
    if (pr != PACK_RESULT::OK)
    {
        LOG_ERROR("Failed to open pack file.");
    }
    else
    {
        uint64 filesCount = g_pVfsReader->GetFileCount();
        LOG_SUCCESS("Pack file opened successfully. %lu files found.", filesCount);
    }
    */

    g_pMainThreadPool.reset(new Common::ThreadPool);

    // init renderer
    LOG_INFO("Initializing renderer...");
    gRenderer.reset(new Renderer::HighLevelRenderer());
    gRenderer->Init("nfRendererD3D11");

#ifdef USE_ANT_TWEAK
    InitAntTweakBar();
#endif

    //init resource manager
    LOG_INFO("Initializing resources manager...");
    g_pResManager = new ResManager;

    g_Timer.Start();
    g_Initialized = true;
    return Result::OK;
}

Result EngineRelease()
{
    if (!g_Initialized)
        return Result::AlreadyFree;

    //destroy scenes
    if (!g_Scenes.empty())
    {
        //delete all scenes
        for (auto pScene : g_Scenes)
            delete pScene;
        g_Scenes.clear();

        LOG_WARNING("Not all scenes has been released before engine shutdown.");
    }

#ifdef USE_ANT_TWEAK
    //free AntTweakBar library
    TwTerminate();
#endif

    //release resources manager
    if (g_pResManager != NULL)
    {
        delete g_pResManager;
        g_pResManager = 0;
    }
    LOG_INFO("Resources manager released.");


    /// release renderer
    gRenderer.reset();
    LOG_INFO("Renderer released.");

    g_pMainThreadPool.reset();
    LOG_INFO("Main threadpool released.");

    g_Initialized = false;
    return Result::OK;
}

Result EngineAdvance(const DrawRequest* pDrawRequests, uint32 drawRequestsNum,
                     const UpdateRequest* pUpdateRequests, uint32 updateRequestsNum)
{
    using namespace Util;

    //renderer is not initialized
    if (!gRenderer)
        return Result::NotInitialized;

    //check 'pDrawRequests' array
    if (!(Common::MemoryCheck(pDrawRequests, sizeof(DrawRequest)*drawRequestsNum) & ACCESS_READ))
    {
        LOG_ERROR("'pDrawRequests' pointer is corrupted.");
        return Result::CorruptedPointer;
    }

    //check 'pUpdateRequests' array
    if (!(Common::MemoryCheck(pUpdateRequests, sizeof(UpdateRequest)*updateRequestsNum) & ACCESS_READ))
    {
        LOG_ERROR("'pUpdateRequests' pointer is corrupted.");
        return Result::CorruptedPointer;
    }

    Util::g_FrameStats.Reset();


    // update physics
    for (uint32 i = 0; i < updateRequestsNum; i++)
    {
        SceneManager* pScene = pUpdateRequests[i].pScene;
        //check if scene is valid
        if (g_Scenes.count(pScene) == 0)
        {
            LOG_ERROR("Scene pointer '%p' passed in XFrameRequest structure is invalid - the scene does not exist.",
                      pScene);
            return Result::Error;
        }
        pScene->Update(pUpdateRequests[i].deltaTime);
    }

    for (uint32 i = 0; i < drawRequestsNum; i++)
    {
        // TODO: error checking
        View* pView = pDrawRequests[i].pView;
        if (pView == NULL) continue;

        Camera* pCamera = pView->GetCamera();
        if (pCamera != NULL)
        {
            SceneManager* pScene = pCamera->GetOwner()->GetScene();
            pScene->Render(pCamera, pView->GetRenderTarget());
        }

        gRenderer->ProcessView(pView);

        RenderContext* ctx = gRenderer->GetImmediateContext();
        GuiRenderer::Get()->Enter(ctx);
        {
            Recti rect;
            GuiRenderer::Get()->SetTarget(ctx, pView->GetRenderTarget());
            pView->OnPostRender(ctx);

            // draw sign
            rect = Recti(6, 6, 100, 100);
            GuiRenderer::Get()->PrintTextWithBorder(ctx, "NFEngine v0.1 [Demo]", rect, 0, 0,
                                                    0xFFFFFFFF, 0x60000000);
        }
        GuiRenderer::Get()->Leave(ctx);

#ifdef USE_ANT_TWEAK
        TwDraw();
#endif

        // present frame in the display
        pView->Present();
    }

    Util::g_FrameStats.deltaTime = g_Timer.Stop();
    g_Timer.Start();
    return Result::OK;
}

ResManager* EngineGetResManager()
{
    return g_pResManager;
}

HighLevelRenderer* EngineGetRenderer()
{
    return gRenderer.get();
}

ResourceBase* EngineGetResource(ResourceType resType, const char* pResName, bool check)
{
    if (g_pResManager)
        return g_pResManager->GetResource(pResName, resType, check);

    return 0;
}

Result EngineAddResource(ResourceBase* pResource, const char* pName)
{
    if (g_pResManager)
        return g_pResManager->AddCustomResource(pResource, pName);

    return Result::NotInitialized;
}

} // namespace NFE
