/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic engine's API implementation.
 */

#include "PCH.hpp"
#include "Engine.hpp"
#include "Resources/Texture.hpp"
#include "Systems/RendererSystem.hpp"

#include "Renderer/HighLevelRenderer.hpp"
#include "Renderer/GuiRenderer.hpp"
#include "Renderer/View.hpp"
#include "Renderer/Font.hpp"

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

    Font::InitFreeType();

    // init renderer
    LOG_INFO("Initializing renderer...");
    mRenderer.reset(new Renderer::HighLevelRenderer());
    if (!mRenderer->Init("nfRendererD3D11"))
    {
        mRenderer.reset();
        LOG_ERROR("Failed to initialize renderer. Drawing will not be supported");
    }

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

    // release renderer modules before threadpool - shaders are resources,
    // so they need to be released first
    mRenderer->ReleaseModules();
    LOG_INFO("Renderer modules released.");

    // release resources manager
    mResManager.Release();
    LOG_INFO("Resources manager released.");

    // release renderer
    mRenderer.reset();
    LOG_INFO("Renderer released.");

    Font::ReleaseFreeType();
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

bool Engine::Advance(View** views, size_t viewsNum,
                     const UpdateRequest* updateRequests, size_t updateRequestsNum)
{
    using namespace Util;

    if (viewsNum > 0 && !mRenderer)
    {
        LOG_ERROR("Renderer is not initialized, drawing is not supported.");
        return false;
    }

    std::unique_lock<std::mutex> lock(mRenderingMutex);

    // update physics
    bool scenesUpdatedSuccessfully = true;
    for (size_t i = 0; i < updateRequestsNum; i++)
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

    // temporary rendering data for each view
    std::vector<RenderingData, Common::AlignedAllocator<RenderingData, 16>> renderingData;
    renderingData.resize(viewsNum);

    bool scenesRenderedSuccessfully = true;
    for (size_t i = 0; i < viewsNum; i++)
    {
        View* view = views[i];
        if (view == nullptr)
            continue;
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
            renderingData[i].view = view;
            scene->Render(renderingData[i]);
        }
    }

    for (size_t i = 0; i < viewsNum; i++)
    {
        View* view = renderingData[i].view;
        if (view == nullptr)
            continue;

        // FIXME
        renderingData[i].ExecuteCommandLists();

        RenderContext* ctx = mRenderer->GetDefaultContext();
        ctx->commandBuffer->Reset();
        view->Postprocess(ctx);
        view->UpdateGui();

        // GUI renderer pass
        {
            GuiRenderer::Get()->Enter(ctx);
            GuiRenderer::Get()->SetTarget(ctx, view->GetRenderTarget(true));
            view->DrawGui(ctx);
            GuiRenderer::Get()->BeginOrdinaryGuiRendering(ctx);
            view->OnPostRender(ctx);
            GuiRenderer::Get()->Leave(ctx);
        }

        mRenderer->GetDevice()->Execute(ctx->commandBuffer->Finish().get());

        // present frame in the display
        view->Present();
    }

    mMainThreadPool.WaitForAllTasks();

    return scenesUpdatedSuccessfully && scenesRenderedSuccessfully;
}

} // namespace NFE
