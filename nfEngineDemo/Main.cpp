#include "PCH.hpp"
#include "Main.hpp"
#include "Scenes.hpp"
#include "GameWindow.hpp"

#include "nfCore/Resources/CollisionShape.hpp"

#include "nfCommon/System/Window.hpp"
#include "nfCommon/System/Assertion.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"
#include "nfCommon/System/KeyCodes.hpp"

namespace NFE {

using namespace Renderer;
using namespace Math;
using namespace Scene;
using namespace Resource;

Common::UniquePtr<Font> gFont;
Common::DynArray<Common::UniquePtr<GameWindow>> gWindows;
Engine* gEngine = nullptr;
float gDeltaTime = 0.0f;

GameWindow* AddWindow(GameWindow* parent)
{
    auto window = Common::MakeUniquePtr<GameWindow>();
    window->Init();
    window->SetSize(1280, 720);
    window->SetTitle("NFEngine Demo");
    window->Open();
    window->SetUpScene(0, parent);

    GameWindow* windowPtr = window.Get();
    gWindows.PushBack(std::move(window));
    return windowPtr;
}

// temporary
bool OnLoadCustomShapeResource(ResourceBase* res, void* data)
{
    CollisionShape* shape = (CollisionShape*)res;

    if (strcmp(shape->GetName(), "shape_floor") == 0)
    {
        shape->AddBox(Vector4(100.0f, 10.0f, 100.0f), Matrix4());
    }
    else if (strcmp(shape->GetName(), "shape_box") == 0)
    {
        shape->AddBox(Vector4(0.25f, 0.25f, 0.25f), Matrix4());
    }
    else if (strcmp(shape->GetName(), "shape_frame") == 0)
    {
        // Z axis
        shape->AddBox(Vector4(0.025f, 0.025f, 0.45f), Matrix4::MakeTranslation3(Vector4(-0.475f, -0.475f, 0.0f)));
        shape->AddBox(Vector4(0.025f, 0.025f, 0.45f), Matrix4::MakeTranslation3(Vector4(-0.475f, 0.475f, 0.0f)));
        shape->AddBox(Vector4(0.025f, 0.025f, 0.45f), Matrix4::MakeTranslation3(Vector4(0.475f, -0.475f, 0.0f)));
        shape->AddBox(Vector4(0.025f, 0.025f, 0.45f), Matrix4::MakeTranslation3(Vector4(0.475f, 0.475f, 0.0f)));

        // Y axis
        shape->AddBox(Vector4(0.025f, 0.5f, 0.025f), Matrix4::MakeTranslation3(Vector4(-0.475f, 0.0f, -0.475f)));
        shape->AddBox(Vector4(0.025f, 0.5f, 0.025f), Matrix4::MakeTranslation3(Vector4(-0.475f, 0.0f, 0.475f)));
        shape->AddBox(Vector4(0.025f, 0.5f, 0.025f), Matrix4::MakeTranslation3(Vector4(0.475f, 0.0f, -0.475f)));
        shape->AddBox(Vector4(0.025f, 0.5f, 0.025f), Matrix4::MakeTranslation3(Vector4(0.475f, 0.0f, 0.475f)));

        // X axis
        shape->AddBox(Vector4(0.5f, 0.025f, 0.025f), Matrix4::MakeTranslation3(Vector4(0.0f, -0.475f, -0.475f)));
        shape->AddBox(Vector4(0.5f, 0.025f, 0.025f), Matrix4::MakeTranslation3(Vector4(0.0f, -0.475f, 0.475f)));
        shape->AddBox(Vector4(0.5f, 0.025f, 0.025f), Matrix4::MakeTranslation3(Vector4(0.0f, 0.475f, -0.475f)));
        shape->AddBox(Vector4(0.5f, 0.025f, 0.025f), Matrix4::MakeTranslation3(Vector4(0.0f, 0.475f, 0.475f)));
    }
    else if (strcmp(shape->GetName(), "shape_barrel") == 0)
    {
        shape->AddCylinder(1.31f, 0.421f);
    }

    return true;
}

void InitializeCustomResources()
{
    gFont = Common::MakeUniquePtr<Font>();
    gFont->Init("nfEngineDemo/Data/Fonts/Inconsolata.otf", 11);

    CollisionShape* floorShape = ENGINE_GET_COLLISION_SHAPE("shape_floor");
    floorShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    floorShape->Load();
    floorShape->AddRef();

    CollisionShape* frameShape = ENGINE_GET_COLLISION_SHAPE("shape_frame");
    frameShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    frameShape->Load();
    frameShape->AddRef();

    CollisionShape* boxShape = ENGINE_GET_COLLISION_SHAPE("shape_box");
    boxShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    boxShape->Load();
    boxShape->AddRef();

    CollisionShape* barrelShape = ENGINE_GET_COLLISION_SHAPE("shape_barrel");
    barrelShape->SetCallbacks(OnLoadCustomShapeResource, NULL);
    barrelShape->Load();
    barrelShape->AddRef();

    CollisionShape* chamberShape = ENGINE_GET_COLLISION_SHAPE("chamber_collision_shape.nfcs");
    chamberShape->Load();
    chamberShape->AddRef();
}

void MainLoop()
{
    Common::DynArray<GameWindow*> windows;
    Common::DynArray<UpdateRequest> updateRequests;
    Common::DynArray<View*> drawRequests;
    Common::Timer timer;
    timer.Start();

    while (!gWindows.Empty())
    {
        // measure delta time
        gDeltaTime = static_cast<float>(timer.Stop());
        timer.Start();

        char str[128];
        sprintf(str, "NFEngine Demo  -  Press [0-%i] to switch scene", GetScenesNum() - 1);

        // work on copy of gWindows
        windows.Clear();
        for (auto& window : gWindows)
            windows.PushBack(window.Get());

        // refresh all the windows
        updateRequests.Clear();
        drawRequests.Clear();
        for (GameWindow* window : windows)
        {
            window->SetTitle(str);
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

            // Build list of scene update requests.
            // They should not be duplicated (multiple windows can share the same scene).
            auto it = std::find_if(updateRequests.begin(), updateRequests.end(), [&](const UpdateRequest& request)
            {
                return request.scene == window->GetScene();
            });

            if (it == updateRequests.end())
            {
                UpdateRequest request;
                request.scene = window->GetScene();
                request.deltaTime = gDeltaTime;
                updateRequests.PushBack(request);
            }

            drawRequests.PushBack(window->GetView());
        }

        // advance the engine
        gEngine->Advance(drawRequests, updateRequests);
    }
}

} // namespace NFE


// application entry point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    using namespace NFE;

    const Common::String execPath = Common::FileSystem::GetExecutablePath();
    const Common::String execDir = Common::FileSystem::GetParentDir(execPath);
    Common::FileSystem::ChangeDirectory(execDir + "/../../..");

    // initialize engine
    gEngine = Engine::GetInstance();
    if (gEngine == nullptr)
        return 1;

    // load manual resources
    InitializeCustomResources();

    // spawn default window
    AddWindow();

    // game loop
    MainLoop();

    // cleanup
    gFont.Reset();
    Engine::Release();

    NFE::Common::ShutdownSubsystems();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return 0;
}
