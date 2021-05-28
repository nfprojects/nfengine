#include "PCH.hpp"
#include "Main.hpp"
#include "GameWindow.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Common/System/Window.hpp"
#include "Engine/Common/System/Assertion.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/FileSystem/FileSystem.hpp"
#include "Engine/Common/System/KeyCodes.hpp"

namespace NFE {

using namespace Math;
using namespace Resource;

Common::DynArray<Common::UniquePtr<GameWindow>> gWindows;
Engine* gEngine = nullptr;
float gDeltaTime = 0.0f;

GameWindow* AddWindow(GameWindow* parent)
{
    auto window = Common::MakeUniquePtr<GameWindow>();
    window->Init();
    window->SetSize(1920, 1080);
    window->SetTitle("NFEngine Demo");
    window->Open();
    window->InitViewport();
    window->SetUpScene(parent);

    GameWindow* windowPtr = window.Get();
    gWindows.PushBack(std::move(window));
    return windowPtr;
}

void MainLoop()
{
    Renderer::Renderer& renderer = Renderer::Renderer::GetInstance();

    Common::DynArray<GameWindow*> windows;
    Common::Timer timer;
    timer.Start();

    while (!gWindows.Empty())
    {
        // measure delta time
        gDeltaTime = static_cast<float>(timer.Stop());
        timer.Start();

        // work on copy of gWindows
        windows.Clear();
        for (auto& window : gWindows)
            windows.PushBack(window.Get());

        // refresh all the windows
        for (GameWindow* window : windows)
        {
            window->ProcessMessages();

            // remove if closed
            if (window->IsClosed())
            {
                auto it = std::find_if(gWindows.begin(), gWindows.end(), [&](const Common::UniquePtr<GameWindow>& w)
                {
                    return w.Get() == window;
                });

                if (it != gWindows.end())
                {
                    gWindows.Erase(it);
                }
                continue;
            }

            Scene::SceneUpdateInfo sceneUpdateInfo;
            sceneUpdateInfo.timeDelta = gDeltaTime;
            window->GetScene()->Update(sceneUpdateInfo);

            Renderer::WindowViewport* viewport = window->GetViewport();

            Renderer::FrameInfo frameInfo;
            frameInfo.viewport = viewport;
            renderer.DrawFrame(frameInfo);

            //// Build list of scene update requests.
            //// They should not be duplicated (multiple windows can share the same scene).
            //auto it = std::find_if(updateRequests.begin(), updateRequests.end(), [&](const UpdateRequest& request)
            //{
            //    return request.scene == window->GetScene();
            //});

            //if (it == updateRequests.end())
            //{
            //    UpdateRequest request;
            //    request.scene = window->GetScene();
            //    request.deltaTime = gDeltaTime;
            //    updateRequests.PushBack(request);
            //}

            //drawRequests.PushBack(window->GetView());
        }

        // advance the engine
        //gEngine->Advance(drawRequests, updateRequests);
    }
}

} // namespace NFE


// application entry point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    using namespace NFE;

    if (!Common::InitSubsystems())
    {
        NFE::Common::ShutdownSubsystems();
        return -1;
    }

    const Common::String execPath = Common::FileSystem::GetExecutablePath();
    const Common::StringView execDir = Common::FileSystem::GetParentDir(execPath);
    Common::FileSystem::ChangeDirectory(execDir + "/../../..");

    Renderer::Renderer& renderer = Renderer::Renderer::GetInstance();
    if (!renderer.Initialize())
    {
        return -1;
    }

    // spawn default window
    AddWindow();

    // game loop
    MainLoop();

    gWindows.Clear();

    renderer.Shutdown();

    NFE::Common::ShutdownSubsystems();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return 0;
}
