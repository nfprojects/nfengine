/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic engine's API implementation.
 */

#include "PCH.hpp"
#include "Engine.hpp"
#include "Texture.hpp"
#include "Performance.hpp"
#include "Systems/RendererSystem.hpp"

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

std::unique_ptr<Engine> gEngineInstance;

Engine::Engine()
{
}

bool Engine::OnInit()
{
    // TODO: Use SystemInfo class to check if hardware is good enough or to monitor memory usage

    // init renderer
    LOG_INFO("Initializing renderer...");
    mRenderer.reset(new Renderer::HighLevelRenderer());
    if (!mRenderer->Init("nfRendererD3D11"))
    {
        mRenderer.reset();
        LOG_ERROR("Failed to initialize renderer. Drawing will not be supported");
    }

#ifdef USE_ANT_TWEAK
    // TODO: support for OpenGL renderer
    int err = TwInit(TW_DIRECT3D11, mRenderer->GetDevice()->GetHandle());
    if (err != 1)
    {
        LOG_ERROR("Failed to initialize AntTweakBar.");
    }
    else
    {
        TwDefine("GLOBAL contained=true"); // bars cannot move outside of the window
        LOG_SUCCESS("AntTweakBar initialized.");
    }
#endif

    return true;
}

void Engine::OnRelease()
{
    // destroy scenes
    if (!mScenes.empty())
    {
        for (auto& scene : mScenes)
            delete scene;

        mScenes.clear();
        LOG_WARNING("Not all scenes has been released before engine shutdown.");
    }

#ifdef USE_ANT_TWEAK
    // free AntTweakBar library
    TwTerminate();
#endif

    // release resources manager
    mResManager.Release();
    LOG_INFO("Resources manager released.");

    // release renderer
    mRenderer.reset();
    LOG_INFO("Renderer released.");
}

SceneManager* Engine::CreateScene()
{
    SceneManager* scene = new SceneManager;
    if (scene == nullptr)
    {
        LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    mScenes.insert(scene);
    return scene;
}


void Engine::DeleteScene(SceneManager* scene)
{
    if (mScenes.erase(scene))
    {
        delete scene;
        LOG_WARNING("Scene has been deleted"); // TODO: which scene?
        return;
    }

    LOG_WARNING("Scene not found, scene = %p", scene);
}

Engine* Engine::GetInstance()
{
    if (gEngineInstance)
        return gEngineInstance.get();

    gEngineInstance.reset(new (std::nothrow) Engine);
    if (!gEngineInstance)
    {
        LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    if (!gEngineInstance->OnInit())
    {
        gEngineInstance.reset();
        return nullptr;
    }

    return gEngineInstance.get();
}

void Engine::Release()
{
    if (!gEngineInstance)
    {
        LOG_ERROR("The engine is already released");
        return;
    }

    gEngineInstance->OnRelease();
    gEngineInstance.reset();
}

bool Engine::Advance(const DrawRequest* drawRequests, uint32 drawRequestsNum,
                     const UpdateRequest* updateRequests, uint32 updateRequestsNum)
{
    using namespace Util;

    if (drawRequestsNum > 0 && !mRenderer)
    {
        LOG_ERROR("Renderer is not initialized, drawing is not supported.");
        return false;
    }

    Util::g_FrameStats.Reset();

    std::unique_lock<std::mutex> lock(mRenderingMutex);

    // update physics
    bool scenesUpdatedSuccessfully = true;
    for (uint32 i = 0; i < updateRequestsNum; i++)
    {
        SceneManager* scene = updateRequests[i].scene;

        //check if scene is valid
        if (mScenes.count(scene) == 0)
        {
            LOG_ERROR("Scene pointer '%p' passed in UpdateRequest structure is invalid "
                      "- the scene does not exist.", scene);
            scenesUpdatedSuccessfully = false;
            continue;
        }

        scene->Update(updateRequests[i].deltaTime);
    }

    bool scenesRenderedSuccessfully = true;
    for (uint32 i = 0; i < drawRequestsNum; i++)
    {
        // TODO: error checking
        View* view = drawRequests[i].view;
        if (view == nullptr) continue;

        ICommandBuffer* commandBuffer = mRenderer->GetImmediateContext()->commandBuffer;
        SceneManager* scene = view->GetSceneManager();

        //check if scene is valid
        if (mScenes.count(scene) == 0)
        {
            LOG_ERROR("Scene pointer '%p' passed in DrawRequest structure is invalid "
                      "- the scene does not exist.", scene);
            scenesRenderedSuccessfully = false;
            continue;
        }

        if (scene != nullptr)
        {
            // FIXME: temporary
            commandBuffer->SetRenderTarget(view->GetRenderTarget());
            uint32 width, height;
            view->GetSize(width, height);
            commandBuffer->SetViewport(0.0f, static_cast<float>(width),
                                       0.0f, static_cast<float>(height),
                                       0.0f, 1.0f);
            scene->GetRendererSystem()->Render(view);
        }

        mRenderer->ProcessView(view);

        RenderContext* ctx = mRenderer->GetImmediateContext();
        GuiRenderer::Get()->Enter(ctx);
        {

            GuiRenderer::Get()->SetTarget(ctx, view->GetRenderTarget());
            view->OnPostRender(ctx);

            // TODO draw engine signature with current version
        }
        GuiRenderer::Get()->Leave(ctx);

#ifdef USE_ANT_TWEAK
        commandBuffer->BeginDebugGroup("AntTweak");
        TwDraw();
        commandBuffer->EndDebugGroup();
#endif

        // present frame in the display
        view->Present();
    }

    return scenesUpdatedSuccessfully && scenesRenderedSuccessfully;
}

} // namespace NFE
