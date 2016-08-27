#include "PCH.hpp"
#include "Test.hpp"
#include "Editor.hpp"
#include "Scenes.hpp"

#include "nfCommon/Window.hpp"
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
Random gRandom;
float gDeltaTime = 0.0f;

const int SECONDARY_VIEW_WIDTH = 256;
const int SECONDARY_VIEW_HEIGHT = 256;

CustomWindow* AddWindow(CustomWindow* parent = nullptr);

class MainCameraView : public NFE::Renderer::View
{
public:
    bool showViewProperties;
    bool showProfiler;
    bool drawSecondaryView;
    std::string secondaryViewTexName;

    const int dtHistorySize = 100;
    std::vector<float> dtHistory;

    MainCameraView()
        : Renderer::View(/* useImGui = */ true)
        , drawSecondaryView(false)
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
                                                0xFFFFFFFF, 0x40000000,
                                                VerticalAlignment::Bottom);

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
    }
};

void SceneDeleter(Scene::SceneManager* scene)
{
    if (scene->GetEntityManager() == gEntityManager)
    {
        gEntityManager = nullptr;
        gSelectedEntity = -1;
    }

    gEngine->DeleteScene(scene);
}

// overload own callback functions
class CustomWindow : public Common::Window
{
    EntityManager* entityManager;

public:
    Quaternion cameraOrientation;
    EntityID cameraEntity;
    EntityID secondaryCameraEntity;  // camera for secondary view
    std::unique_ptr<MainCameraView> view;
    std::unique_ptr<Renderer::View> secondaryView;
    std::shared_ptr<Scene::SceneManager> scene;

    bool cameraControl;
    float cameraXZ;
    float cameraY;

    void* operator new(size_t size)
    {
        return _aligned_malloc(size, 16);
    }

    void operator delete(void* ptr)
    {
        _aligned_free(ptr);
    }

    CustomWindow()
        : cameraEntity(-1)
        , entityManager(nullptr)
    {
        InitCameraOrientation();
    }

    ~CustomWindow()
    {
        entityManager = nullptr;
    }

    void InitCameraOrientation()
    {
        cameraControl = 0;
        cameraXZ = 0.0f;
        cameraY = 0.0f;
        cameraOrientation = QuaternionMultiply(QuaternionRotationY(cameraXZ),
                                               QuaternionRotationX(-cameraY));
    }

    void SetUpScene(int sceneId = 0, CustomWindow* parent = nullptr)
    {
        if (parent == nullptr)  // init a new scene
        {
            SceneManager* newScene = InitScene(sceneId);
            if (newScene == nullptr)
                return;
            scene.reset(newScene, SceneDeleter);
        }
        else  // fork
        {
            scene = parent->scene;
        }

        entityManager = scene->GetEntityManager();
        InitCamera();
    }

    // create camera entity, etc.
    void InitCamera()
    {
        InitCameraOrientation();
        cameraEntity = entityManager->CreateEntity();

        TransformComponent* transform = entityManager->AddComponent<TransformComponent>(cameraEntity);
        transform->SetPosition(Vector(0.0f, 1.6f, -2.0f, 0.0f));

        UpdateCamera();
        Perspective perspective;
        perspective.FoV = NFE_MATH_PI * 60.0f / 180.0f;
        perspective.nearDist = 0.01f;
        perspective.farDist = 1000.0f;
        perspective.aspectRatio = GetAspectRatio();
        CameraComponent* camera = entityManager->AddComponent<CameraComponent>(cameraEntity);
        camera->SetPerspective(&perspective);

        entityManager->AddComponent<BodyComponent>(cameraEntity);

        if (!view)
        {
            view.reset(new MainCameraView);
            view->SetWindow(this);
        }
        view->SetCamera(scene.get(), cameraEntity);


        InitSecondaryCamera();

        std::string secondaryViewTexName = "secondaryViewTexture_" +
            std::to_string(reinterpret_cast<size_t>(view.get()));
        secondaryView.reset(new Renderer::View);
        secondaryView->SetCamera(scene.get(), secondaryCameraEntity);
        secondaryView->SetOffScreen(SECONDARY_VIEW_WIDTH, SECONDARY_VIEW_HEIGHT,
                                    secondaryViewTexName.c_str());

        // set custom postprocessing parameters
        secondaryView->postProcessParams.noiseFactor = 0.1f;
        secondaryView->postProcessParams.saturation = 0.5f;

        view->secondaryViewTexName = secondaryViewTexName;
    }

    void InitSecondaryCamera()
    {
        secondaryCameraEntity = entityManager->CreateEntity();

        TransformComponent* transform = entityManager->AddComponent<TransformComponent>(secondaryCameraEntity);
        transform->SetPosition(Vector(0.0f, 1.6f, -2.0f, 0.0f));

        UpdateCamera();
        Perspective perspective;
        perspective.FoV = NFE_MATH_PI * 60.0f / 180.0f;
        perspective.nearDist = 0.01f;
        perspective.farDist = 1000.0f;
        perspective.aspectRatio = static_cast<float>(SECONDARY_VIEW_WIDTH) /
                                  static_cast<float>(SECONDARY_VIEW_HEIGHT);
        CameraComponent* camera = entityManager->AddComponent<CameraComponent>(secondaryCameraEntity);
        camera->SetPerspective(&perspective);
    }

#define CAMERA_ROTATION_SMOOTHING 0.05f
#define CAMERA_TRANSLATION_SMOOTHING 0.2f

    // update camera position and orientation
    void UpdateCamera()
    {
        auto cameraBody = entityManager->GetComponent<BodyComponent>(cameraEntity);
        auto cameraTransform = entityManager->GetComponent<TransformComponent>(cameraEntity);
        if (cameraBody == nullptr || cameraTransform == nullptr)
            return;

        Quaternion destOrientation = QuaternionMultiply(QuaternionRotationY(cameraXZ),
                                     QuaternionRotationX(-cameraY));
        destOrientation = QuaternionNormalize(destOrientation);

        //LPF
        Quaternion prevOrientation = cameraOrientation;
        cameraOrientation = QuaternionInterpolate(cameraOrientation, destOrientation,
                            gDeltaTime / (CAMERA_ROTATION_SMOOTHING + gDeltaTime));
        cameraOrientation = QuaternionNormalize(cameraOrientation);

        Quaternion rotation = QuaternionMultiply(prevOrientation,
                                                 QuaternionInverse(cameraOrientation));
        cameraBody->SetAngularVelocity(-QuaternionToAxis(rotation) / gDeltaTime);

        Matrix rotMatrix = MatrixFromQuaternion(QuaternionNormalize(cameraOrientation));
        Orientation orient;
        orient.x = rotMatrix.r[0];
        orient.y = rotMatrix.r[1];
        orient.z = rotMatrix.r[2];
        cameraTransform->SetOrientation(orient);


        Vector destVelocity = Vector();
        if (cameraControl)
        {
            if (IsKeyPressed(Common::KeyCode::W)) destVelocity += orient.z;
            if (IsKeyPressed(Common::KeyCode::S)) destVelocity -= orient.z;
            if (IsKeyPressed(Common::KeyCode::D)) destVelocity += orient.x;
            if (IsKeyPressed(Common::KeyCode::A)) destVelocity -= orient.x;
            if (IsKeyPressed(Common::KeyCode::R)) destVelocity += orient.y;
            if (IsKeyPressed(Common::KeyCode::F)) destVelocity -= orient.y;

            if (IsKeyPressed(Common::KeyCode::ShiftLeft)) destVelocity *= 30.0f;
            else if (IsKeyPressed(Common::KeyCode::ControlLeft)) destVelocity *= 0.2f;
            else destVelocity *= 3.0f;
        }

        Vector prevVelocity = cameraBody->GetVelocity();

        // low pass filter - for smooth camera movement
        float factor = gDeltaTime / (CAMERA_TRANSLATION_SMOOTHING + gDeltaTime);
        cameraBody->SetVelocity(VectorLerp(prevVelocity, destVelocity, factor));
    }

    void OnKeyPress(Common::KeyCode key) override
    {
        // create input event structure for the engine
        Utils::KeyPressedEvent event;
        event.key = key;
        event.isAltPressed = IsKeyPressed(Common::KeyCode::AltLeft);
        event.isCtrlPressed = IsKeyPressed(Common::KeyCode::ControlLeft);
        event.isShiftPressed = IsKeyPressed(Common::KeyCode::ShiftLeft);

        if (view && view->OnKeyPressed(event))
            return; // key press event was consumed by the engine

        if (key == Common::KeyCode::F1)
        {
            BOOL fullscreen = GetFullscreenMode();
            SetFullscreenMode(!fullscreen);
        }

        auto cameraTransform = entityManager->GetComponent<TransformComponent>(cameraEntity);
        Orientation orient;

        //place spot light
        if (key == Common::KeyCode::T && cameraTransform != nullptr)
        {
            EntityID lightEntity = entityManager->CreateEntity();
            gSelectedEntity = lightEntity;
            gEntityManager = entityManager;

            TransformComponent* transform = entityManager->AddComponent<TransformComponent>(lightEntity);
            transform->SetOrientation(cameraTransform->GetOrientation());
            transform->SetPosition(cameraTransform->GetPosition());

            LightComponent* light = entityManager->AddComponent<LightComponent>(lightEntity);
            SpotLightDesc lightDesc;
            lightDesc.nearDist = 0.1f;
            lightDesc.farDist = 500.0f;
            lightDesc.cutoff = NFE_MATH_PI / 4.0f;
            lightDesc.maxShadowDistance = 60.0f;
            light->SetSpotLight(&lightDesc);
            light->SetColor(Float3(30.0f, 15.0f, 5.0f));
            light->SetLightMap("flashlight.jpg");
            light->SetShadowMap(1024);
        }

        //place omni light
        if (key == Common::KeyCode::O && cameraTransform != nullptr)
        {
            OmniLightDesc lightDesc;
            lightDesc.radius = 20.0f;
            lightDesc.shadowFadeStart = 20.0f;
            lightDesc.maxShadowDistance = 30.0f;

            EntityID lightEntity = entityManager->CreateEntity();
            gSelectedEntity = lightEntity;
            gEntityManager = entityManager;

            TransformComponent* transform = entityManager->AddComponent<TransformComponent>(lightEntity);
            transform->SetPosition(cameraTransform->GetPosition());

            LightComponent* light = entityManager->AddComponent<LightComponent>(lightEntity);
            light->SetOmniLight(&lightDesc);
            light->SetColor(Float3(25.0f, 25.0f, 25.0f));
            light->SetShadowMap(512);
        }

        if (key == Common::KeyCode::V)
        {
            view->drawSecondaryView ^= true;
        }

        // set secondary camera transform to the primary camera transform
        if (key == Common::KeyCode::C)
        {
            TransformComponent* camTransform =
                entityManager->GetComponent<TransformComponent>(cameraEntity);
            TransformComponent* secondaryCamTransform =
                entityManager->GetComponent<TransformComponent>(secondaryCameraEntity);

            if (camTransform && secondaryCamTransform)
            {
                Matrix matrix = camTransform->GetMatrix();
                secondaryCamTransform->SetMatrix(matrix);
            }
        }

        if (key >= Common::KeyCode::Num0 && key <= Common::KeyCode::Num9)
            SetUpScene(static_cast<unsigned int>(key) -
                       static_cast<unsigned int>(Common::KeyCode::Num0));

        // spawn a new window
        if (key == Common::KeyCode::N)
            AddWindow(this);
    }

    void OnMouseDown(UINT button, int x, int y) override
    {
        // create input event structure for the engine
        Utils::MouseButtonEvent event;
        event.mouseButton = button;
        event.x = x;
        event.y = y;

        if (view && view->OnMouseDown(event))
            return; // mouse event was consumed by the engine

        if (button == 0)
            cameraControl = true;

        auto cameraTransform = entityManager->GetComponent<TransformComponent>(cameraEntity);
        if (cameraTransform == nullptr)
            return;
        Orientation camOrient = cameraTransform->GetOrientation();

        //shoot a cube
        if (button == 1)
        {
            EntityID cube = entityManager->CreateEntity();

            TransformComponent* transform = entityManager->AddComponent<TransformComponent>(cube);
            transform->SetPosition(cameraTransform->GetPosition() + camOrient.z);

            MeshComponent* mesh = entityManager->AddComponent<MeshComponent>(cube);
            mesh->SetMeshResource("cube.nfm");

            BodyComponent* body = entityManager->AddComponent<BodyComponent>(cube);
            body->SetMass(10.0f);
            body->SetVelocity(0.1f * camOrient.z);
            body->EnablePhysics(ENGINE_GET_COLLISION_SHAPE("shape_box"));

            {
                EntityID child = entityManager->CreateEntity();
                gSelectedEntity = cube;
                gEntityManager = entityManager;

                TransformComponent* transform = entityManager->AddComponent<TransformComponent>(child);
                transform->SetLocalPosition(Vector(0.0f, 1.0f, 0.0f));

                OmniLightDesc lightDesc;
                lightDesc.radius = 4.0f;
                lightDesc.shadowFadeStart = 20.0;
                lightDesc.maxShadowDistance = 30.0;
                LightComponent* light = entityManager->AddComponent<LightComponent>(child);
                light->SetOmniLight(&lightDesc);
                light->SetColor(Float3(0.5f, 0.5f, 3.0f));
                light->SetShadowMap(0);

                scene->GetTransformSystem()->SetParent(child, cube);
            }
        }

        if (button == 2)
        {
            EntityID barrel = entityManager->CreateEntity();
            gSelectedEntity = barrel;
            gEntityManager = entityManager;

            TransformComponent* transform = entityManager->AddComponent<TransformComponent>(barrel);
            transform->SetPosition(cameraTransform->GetPosition() + camOrient.z);

            MeshComponent* mesh = entityManager->AddComponent<MeshComponent>(barrel);
            mesh->SetMeshResource("barrel.nfm");

            BodyComponent* body = entityManager->AddComponent<BodyComponent>(barrel);
            body->SetMass(20.0f);
            body->SetVelocity(30.0f * camOrient.z);
            body->EnablePhysics(ENGINE_GET_COLLISION_SHAPE("shape_barrel"));
        }
    }

    void OnMouseMove(int x, int y, int deltaX, int deltaY) override
    {
        // create input event structure for the engine
        Utils::MouseMoveEvent event;
        event.x = x;
        event.y = y;

        if (view && view->OnMouseMove(event))
            return; // mouse event was consumed by the engine

        if (cameraControl)
        {
            float fDeltaX = static_cast<float>(deltaX) * 0.005f;
            float fDeltaY = static_cast<float>(deltaY) * 0.005f;

            cameraXZ += fDeltaX;
            cameraY -= fDeltaY;
            cameraXZ = fmodf(cameraXZ, 2.0f * NFE_MATH_PI);

            if (cameraY > NFE_MATH_PI / 2.0f) cameraY = NFE_MATH_PI / 2.0f;
            if (cameraY < -NFE_MATH_PI / 2.0f) cameraY = -NFE_MATH_PI / 2.0f;
        }
    }

    void OnMouseUp(UINT button) override
    {
        // create input event structure for the engine
        Utils::MouseButtonEvent event;
        event.mouseButton = button;
        GetMousePosition(event.x, event.y);

        if (view && view->OnMouseUp(event))
            return; // mouse event was consumed by the engine

        if (button == 0)
            cameraControl = false;
    }

    void OnScroll(int delta) override
    {
        if (view && view->OnMouseScroll(delta))
            return; // mouse event was consumed by the engine
    }

    void OnCharTyped(const char* charUTF8) override
    {
        if (view && view->OnCharTyped(charUTF8))
            return; // mouse event was consumed by the engine
    }

    // window resized
    void OnResize(UINT width, UINT height)
    {
        if (entityManager == nullptr)
            return;

        auto camera = entityManager->GetComponent<CameraComponent>(cameraEntity);
        if (camera != nullptr)
        {
            if (width || height)
            {
                Perspective perspective;
                camera->GetPerspective(&perspective);
                perspective.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
                camera->SetPerspective(&perspective);
            }
        }
    }
};

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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    std::string execPath = Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");

    //initialize engine
    gEngine = Engine::GetInstance();
    if (gEngine == nullptr)
        return 1;

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

    AddWindow();

    // message loop

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
            window->UpdateCamera();

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
                                   { return request.scene == window->scene.get(); });
            if (it == updateRequests.end())
            {
                UpdateRequest request;
                request.scene = window->scene.get();
                request.deltaTime = gDeltaTime;
                updateRequests.push_back(request);
            }

            // secondary view draw request
            if (window->view->drawSecondaryView)
                drawRequests.push_back(window->secondaryView.get());

            drawRequests.push_back(window->view.get());
        }

        gEngine->Advance(drawRequests.data(), drawRequests.size(),
                         updateRequests.data(), updateRequests.size());
    }

    gFont.reset();
    gWindows.clear();
    Engine::Release();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return 0;
}
