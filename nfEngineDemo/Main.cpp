#include "PCH.hpp"
#include "Main.hpp"
#include "Scenes.hpp"
#include "GameWindow.hpp"
#include "FreeCameraController.hpp"

#include "nfCore/Scene/Systems/InputSystem.hpp"

#include "nfCommon/Window.hpp"
#include "nfCommon/Assertion.hpp"
#include "nfCommon/Logger.hpp"
#include "nfCommon/FileSystem.hpp"
#include "nfCommon/KeyCodes.hpp"


namespace NFE {

using namespace Renderer;
using namespace Math;
using namespace Scene;
using namespace Resource;


std::unique_ptr<Font> gFont;
std::vector<std::unique_ptr<GameWindow>> gWindows;
Engine* gEngine = nullptr;
float gDeltaTime = 0.0f;

void SceneDeleter(Scene::SceneManager* scene)
{
    gEngine->DeleteScene(scene);
}

GameWindow* AddWindow(GameWindow* parent)
{
    std::unique_ptr<GameWindow> window(new GameWindow);
    window->Init();
    window->SetSize(800, 600);
    window->SetTitle("NFEngine Demo");
    window->Open();

    window->SetUpScene(0, parent);
    window->InitCamera();

    GameWindow* windowPtr = window.get();
    gWindows.push_back(std::move(window));
    return windowPtr;
}

// temporary
bool OnLoadCustomShapeResource(ResourceBase* res, void* data)
{
    CollisionShape* shape = (CollisionShape*)res;

    if (strcmp(shape->GetName(), "shape_floor") == 0)
    {
        shape->AddBox(Vector(100.0f, 10.0f, 100.0f), Matrix());
    }
    else if (strcmp(shape->GetName(), "shape_box") == 0)
    {
        shape->AddBox(Vector(0.25f, 0.25f, 0.25f), Matrix());
    }
    else if (strcmp(shape->GetName(), "shape_frame") == 0)
    {
        // Z axis
        shape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(-0.475f, -0.475f, 0.0f)));
        shape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(-0.475f, 0.475f, 0.0f)));
        shape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(0.475f, -0.475f, 0.0f)));
        shape->AddBox(Vector(0.025f, 0.025f, 0.45f), MatrixTranslation3(Vector(0.475f, 0.475f, 0.0f)));

        // Y axis
        shape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(-0.475f, 0.0f, -0.475f)));
        shape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(-0.475f, 0.0f, 0.475f)));
        shape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(0.475f, 0.0f, -0.475f)));
        shape->AddBox(Vector(0.025f, 0.5f, 0.025f), MatrixTranslation3(Vector(0.475f, 0.0f, 0.475f)));

        // X axis
        shape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, -0.475f, -0.475f)));
        shape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, -0.475f, 0.475f)));
        shape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, 0.475f, -0.475f)));
        shape->AddBox(Vector(0.5f, 0.025f, 0.025f), MatrixTranslation3(Vector(0.0f, 0.475f, 0.475f)));
    }
    else if (strcmp(shape->GetName(), "shape_barrel") == 0)
    {
        shape->AddCylinder(1.31f, 0.421f);
    }

    return true;
}

void InitializeCustomResources()
{
    gFont.reset(new Font);
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
    std::vector<UpdateRequest> updateRequests;
    std::vector<View*> drawRequests;
    Common::Timer timer;
    timer.Start();
    while (!gWindows.empty())
    {
        //measure delta time
        gDeltaTime = static_cast<float>(timer.Stop());
        timer.Start();

        char str[128];
        sprintf(str, "NFEngine Demo  -  Press [0-%i] to switch scene", GetScenesNum() - 1);

        // work on copy of gWindows
        std::vector<GameWindow*> windows;
        for (auto& window : gWindows)
            windows.push_back(window.get());

        updateRequests.clear();
        drawRequests.clear();
        for (auto& window : windows)
        {
            window->SetTitle(str);
            window->ProcessMessages();

            // remove if closed
            if (window->IsClosed())
            {
                auto it = std::find_if(gWindows.begin(), gWindows.end(),
                                       [&](const std::unique_ptr<GameWindow>& w)
                { return w.get() == window; });
                if (it != gWindows.end())
                    gWindows.erase(it);
                continue;
            }

            // build list of scene update requests (they should not be duplicated)
            auto it = std::find_if(updateRequests.begin(), updateRequests.end(),
                                   [&](const UpdateRequest& request)
            { return request.scene == window->GetScene(); });
            if (it == updateRequests.end())
            {
                UpdateRequest request;
                request.scene = window->GetScene();
                request.deltaTime = gDeltaTime;
                updateRequests.push_back(request);
            }

            drawRequests.push_back(window->GetView());
        }

        gEngine->Advance(drawRequests.data(), drawRequests.size(),
                         updateRequests.data(), updateRequests.size());
    }
}

} // namespace NFE

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    using namespace NFE;

    std::string execPath = Common::FileSystem::GetExecutablePath();
    std::string execDir = Common::FileSystem::GetParentDir(execPath);
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
    gFont.reset();
    Engine::Release();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return 0;
}
