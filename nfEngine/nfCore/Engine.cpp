/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic engine's API implementation.
 */

#include "PCH.hpp"
#include "Engine.hpp"
#include "Resources/Texture.hpp"
#include "Renderer/RenderScene.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Entity.hpp"

#include "Renderer/HighLevelRenderer.hpp"
#include "Renderer/GuiRenderer.hpp"
#include "Renderer/View.hpp"
#include "Renderer/Font.hpp"

#include "Utils/ConfigVariable.hpp"

#include "nfCommon/System/Memory.hpp"
#include "nfCommon/System/Window.hpp"
#include "nfCommon/System/Timer.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/System/SystemInfo.hpp"

namespace NFE {

using namespace Common;
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
    if (!mRenderer->Init())
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

SceneManager* Engine::CreateScene(const std::string& name)
{
    auto scene = MakeUniquePtr<SceneManager>(name);
    if (!scene)
    {
        LOG_ERROR("Memory allocation failed");
        return nullptr;
    }

    if (!scene->InitializeSystems())
    {
        LOG_ERROR("Scene creation failed: could not initialize scene systems");
        return nullptr;
    }

    LOG_INFO("Scene '%s' created", scene->GetName().c_str());
    mScenes.push_back(std::move(scene));
    return mScenes.back().Get();
}


bool Engine::DeleteScene(SceneManager* scene)
{
    auto iter = std::find_if(mScenes.begin(), mScenes.end(),
                             [scene](const SceneManagerPtr& ptr) { return ptr.Get() == scene; });
    if (mScenes.end() != iter)
    {
        LOG_INFO("Deleting scene: '%s'", scene->GetName().c_str());
        mScenes.erase(iter);
        return true;
    }

    LOG_ERROR("Scene '%s' not found", scene->GetName().c_str());
    return false;
}

Engine* Engine::GetInstance()
{
    if (gEngineInstance)
        return gEngineInstance.get();

    ConfigManager::GetInstance().Initialize();

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
    if (viewsNum > 0 && !mRenderer)
    {
        LOG_ERROR("Renderer is not initialized, drawing is not supported.");
        return false;
    }

    // TODO get rid of that
    std::unique_lock<std::recursive_mutex> lock(mRenderingMutex);

    // update physics
    bool scenesUpdatedSuccessfully = true;
    for (size_t i = 0; i < updateRequestsNum; i++)
    {
        SceneManager* scene = updateRequests[i].scene;

        //check if scene is valid
        auto iter = std::find_if(mScenes.begin(), mScenes.end(),
                                 [scene](const SceneManagerPtr& ptr) { return scene == ptr.Get(); });
        if (mScenes.end() == iter)
        {
            LOG_ERROR("Scene pointer '%p' passed in UpdateRequest structure is invalid "
                      "- the scene does not exist.", scene);
            scenesUpdatedSuccessfully = false;
            continue;
        }

        // begin scene update
        {
            SceneUpdateInfo info;
            info.timeDelta = updateRequests[i].deltaTime;
            scene->BeginUpdate(info);
        }
    }

    // prepare for rendering
    mRenderer->ResetCommandBuffers();

    std::vector<Common::TaskID> renderingTasks;
    renderingTasks.reserve(viewsNum);

    bool scenesRenderedSuccessfully = true;
    for (size_t i = 0; i < viewsNum; i++)
    {
        View* view = views[i];
        NFE_ASSERT(view, "Invalid view pointer provided");

        Entity* cameraEntity = view->GetCameraEntity();
        if (!cameraEntity)
        {
            LOG_ERROR("Invalid camera");
            scenesRenderedSuccessfully = false;
            continue;
        }

        SceneManager* scene = cameraEntity->GetScene();
        if (!scene)
        {
            LOG_ERROR("Camera entity does not belong to any scene");
            scenesRenderedSuccessfully = false;
            continue;
        }

        // check if scene is valid
        auto iter = std::find_if(mScenes.begin(), mScenes.end(),
                                 [scene](const SceneManagerPtr& ptr) { return scene == ptr.Get(); });
        if (mScenes.end() == iter)
        {
            LOG_ERROR("Scene pointer '%p' passed in DrawRequest structure is invalid "
                      "- the scene does not exist.", scene);
            scenesRenderedSuccessfully = false;
            continue;
        }

        // start the rendering
        const Common::TaskID taskID = scene->BeginRendering(view);
        if (taskID != NFE_INVALID_TASK_ID)
        {
            renderingTasks.push_back(taskID);
        }
    }

    // wait until all command buffers are filled
    // TODO get rid of this - everything must be asynchronous !!!
    mMainThreadPool.WaitForTasks(renderingTasks.data(), renderingTasks.size());
    mRenderer->FinishAndExecuteCommandBuffers();

    for (size_t i = 0; i < viewsNum; i++)
    {
        View* view = views[i];
        if (view == nullptr)
            continue;

        // TODO: post process and GUI renderer can be done on multiple threads
        RenderContext* ctx = mRenderer->GetDeferredContext(0);
        ctx->commandRecorderOnScreen->Begin();
        view->Postprocess(ctx);

        // GUI renderer pass
        {
            GuiRendererContext* guiContext = ctx->guiContext.get();

            GuiRenderer::Get()->OnEnter(guiContext);
            GuiRenderer::Get()->SetTarget(guiContext, view->GetRenderTarget(true));
            view->DrawGui(ctx);
            GuiRenderer::Get()->BeginOrdinaryGuiRendering(guiContext);
            view->OnPostRender(guiContext);
            GuiRenderer::Get()->OnLeave(guiContext);
        }

        mRenderer->GetDevice()->Execute(ctx->commandRecorderOnScreen->Finish());

        // present frame in the display
        view->Present();
    }

    mMainThreadPool.WaitForAllTasks();
    mRenderer->GetDevice()->FinishFrame();

    return scenesUpdatedSuccessfully && scenesRenderedSuccessfully;
}

} // namespace NFE
