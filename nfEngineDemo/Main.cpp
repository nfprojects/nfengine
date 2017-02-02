#include "PCH.hpp"
#include "Main.hpp"
#include "Scenes.hpp"
#include "FreeCameraController.hpp"

#include "nfCore/Scene/Systems/InputSystem.hpp"

#include "nfCommon/Window.hpp"
#include "nfCommon/Assertion.hpp"
#include "nfCommon/Logger.hpp"
#include "nfCommon/FileSystem.hpp"
#include "nfCommon/KeyCodes.hpp"


using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

class CustomWindow;

std::unique_ptr<Font> gFont;
std::vector<std::unique_ptr<CustomWindow>> gWindows;
Engine* gEngine = nullptr;
float gDeltaTime = 0.0f;


CustomWindow* AddWindow(CustomWindow* parent = nullptr);

class MainCameraView : public Renderer::View
{
public:
    bool showViewProperties;
    bool showProfiler;

    const int dtHistorySize = 100;
    std::vector<float> dtHistory;

    MainCameraView()
        : Renderer::View(/* useImGui = */ true)
        , showViewProperties(false)
        , showProfiler(false)
    {
        dtHistory.resize(dtHistorySize);
    }

    void OnDrawImGui(void* state) override
    {
        ImGui::SetInternalState(state);

        // Main menu bar
        if (ImGui::BeginMainMenuBar())
        {
            // TODO: switching default scenes, loading/saving, entity editor, etc.

            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Profiler", nullptr, &showProfiler);
                ImGui::MenuItem("Entity Editor", nullptr, false, false);
                ImGui::MenuItem("View properties", nullptr, &showViewProperties);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (showViewProperties)
            DrawViewPropertiesGui();

        if (showProfiler)
        {
            if (ImGui::Begin("Profiler", nullptr, ImVec2(340, 300), 0.8f))
            {
                ImGui::Text("Delta time: %.2fms", 1000.0f * gDeltaTime);
                ImGui::Text("FPS: %.1f", 1.0f / gDeltaTime);
                ImGui::Separator();

                memmove(dtHistory.data(), dtHistory.data() + 1, sizeof(float) * (dtHistorySize - 1));
                dtHistory[dtHistorySize - 1] = 1000.0f * gDeltaTime;
                ImGui::PlotLines("dt history",
                                 dtHistory.data(), dtHistorySize,  // data
                                 0,  // no offset
                                 nullptr, 0.0f, FLT_MAX, ImVec2(0, 100));
            }
            ImGui::End();
        }
    }

    void OnPostRender(GuiRendererContext* ctx) override
    {
        uint32 viewWidth, viewHeight;
        this->GetSize(viewWidth, viewHeight);

        // print time delta
        static float avgDeltaTime = 0.0f;
        avgDeltaTime = Lerp(avgDeltaTime, gDeltaTime, 0.1f);
        char text[128];
        sprintf(text, "dt = %.2fms", 1000.0f * avgDeltaTime);
        GuiRenderer::Get()->PrintTextWithBorder(ctx, gFont.get(), text,
                                                Recti(8, 20, viewWidth, viewHeight - 8),
                                                0xFFFFFFFF, 0xFF000000,
                                                VerticalAlignment::Bottom);

        // TODO restore this
        /*
        if (!drawSecondaryView)
            return;

        float left = 50.0f;
        float bottom = 50.0f;
        float width = static_cast<float>(SECONDARY_VIEW_WIDTH);
        float height = static_cast<float>(SECONDARY_VIEW_HEIGHT);
        float border = 2.0f;

        // border
        GuiRenderer::Get()->DrawQuad(ctx,
                                     Rectf(left - border, viewHeight - bottom - height - border,
                                           left + width + border, viewHeight - bottom + border),
                                     0xAA000000);

        // draw quad with secondary camera view
        Texture* texture = ENGINE_GET_TEXTURE(secondaryViewTexName.c_str());
        GuiRenderer::Get()->DrawTexturedQuad(ctx,
                                             Rectf(left, viewHeight - bottom - height,
                                                   left + width, viewHeight - bottom),
                                             Rectf(0.0f, 0.0f, 1.0f, 1.0f),
                                             texture->GetRendererTextureBinding(),
                                             0xFFFFFFFF);
        */
    }
};

void SceneDeleter(Scene::SceneManager* scene)
{
    gEngine->DeleteScene(scene);
}

CustomWindow* AddWindow(CustomWindow* parent)
{
    std::unique_ptr<CustomWindow> window(new CustomWindow);
    window->Init();
    window->SetSize(800, 600);
    window->SetTitle("NFEngine Demo");
    window->Open();
    window->SetUpScene(0, parent);

    CustomWindow* windowPtr = window.get();
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
        std::vector<CustomWindow*> windows;
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
                                       [&](const std::unique_ptr<CustomWindow>& w)
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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    std::string execPath = Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");

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
