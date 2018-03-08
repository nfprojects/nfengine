/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic engine's API implementation.
 */

#include "PCH.hpp"
#include "Engine.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Entity.hpp"

#include "Renderer/GuiRenderer.hpp"
#include "Renderer/View.hpp"

#include "Utils/ConfigVariable.hpp"


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
    NFE_LOG_INFO("Initializing renderer...");
    mRenderer = MakeUniquePtr<Renderer::HighLevelRenderer>();
    if (!mRenderer->Init())
    {
        mRenderer.Reset();
        NFE_LOG_ERROR("Failed to initialize renderer. Drawing will not be supported");
        return false;
    }

    return true;
}

void Engine::OnRelease()
{
    // release renderer modules before threadpool - shaders are resources,
    // so they need to be released first
    mRenderer->ReleaseModules();
    NFE_LOG_INFO("Renderer modules released.");

    // release resources manager
    mResManager.Release();
    NFE_LOG_INFO("Resources manager released.");

    // release renderer
    mRenderer.Reset();
    NFE_LOG_INFO("Renderer released.");

    Font::ReleaseFreeType();
}

Engine* Engine::GetInstance()
{
    if (gEngineInstance)
        return gEngineInstance.get();

    ConfigManager::GetInstance().Initialize();

    gEngineInstance.reset(new (std::nothrow) Engine);
    if (!gEngineInstance)
    {
        NFE_LOG_ERROR("Memory allocation failed");
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
        NFE_LOG_ERROR("The engine is already released");
        return;
    }

    gEngineInstance->OnRelease();
    gEngineInstance.reset();
}

bool Engine::Advance(const Common::ArrayView<Renderer::View*> views, const Common::ArrayView<const UpdateRequest> updateRequests)
{
    if (views.Size() > 0 && !mRenderer)
    {
        NFE_LOG_ERROR("Renderer is not initialized, drawing is not supported.");
        return false;
    }

    // TODO get rid of that
    std::unique_lock<std::recursive_mutex> lock(mRenderingMutex);

    // update physics
    bool scenesUpdatedSuccessfully = true;
    for (const UpdateRequest& updateRequest : updateRequests)
    {
        SceneManager* scene = updateRequest.scene;
        NFE_ASSERT(scene, "Invalid scene pointer");

        // TODO check if scene is valid

        // begin scene update
        {
            SceneUpdateInfo info;
            info.timeDelta = updateRequest.deltaTime;
            scene->BeginUpdate(info);
        }
    }

    // prepare for rendering
    mRenderer->ResetCommandBuffers();

    Common::DynArray<Common::TaskID> renderingTasks;
    renderingTasks.Reserve(views.Size());

    bool scenesRenderedSuccessfully = true;
    for (View* view : views)
    {
        NFE_ASSERT(view, "Invalid view pointer provided");

        Entity* cameraEntity = view->GetCameraEntity();
        if (!cameraEntity)
        {
            NFE_LOG_ERROR("Invalid camera");
            scenesRenderedSuccessfully = false;
            continue;
        }

        SceneManager& scene = cameraEntity->GetScene();

        // TODO check if scene is valid

        // start the rendering
        const Common::TaskID taskID = scene.BeginRendering(view);
        if (taskID != NFE_INVALID_TASK_ID)
        {
            renderingTasks.PushBack(taskID);
        }
    }

    // wait until all command buffers are filled
    // TODO get rid of this - everything must be asynchronous !!!
    mMainThreadPool.WaitForTasks(renderingTasks.Data(), renderingTasks.Size());
    mRenderer->FinishAndExecuteCommandBuffers();

    for (View* view : views)
    {
        NFE_ASSERT(view, "Invalid view pointer provided");

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
