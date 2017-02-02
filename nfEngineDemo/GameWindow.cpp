#include "PCH.hpp"
#include "GameWindow.hpp"
#include "Scenes.hpp"
#include "Main.hpp"
#include "FreeCameraController.hpp"

#include "nfCore/Scene/Systems/InputSystem.hpp"

#include "nfCommon/System/Window.hpp"
#include "nfCommon/System/Assertion.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/System/KeyCodes.hpp"


namespace NFE {

using namespace Renderer;
using namespace Math;
using namespace Scene;
using namespace Resource;


MainCameraView::MainCameraView()
    : Renderer::View(/* useImGui = */ true)
    , showViewProperties(false)
{
}

void MainCameraView::OnDrawImGui(void* state)
{
    ImGui::SetInternalState(state);

    // Main menu bar
    if (ImGui::BeginMainMenuBar())
    {
        // TODO: switching default scenes, loading/saving, entity editor, etc.

        if (ImGui::BeginMenu("Tools"))
        {
            ImGui::MenuItem("Entity Editor", nullptr, false, false);
            ImGui::MenuItem("View properties", nullptr, &showViewProperties);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (showViewProperties)
        DrawViewPropertiesGui();
}

void MainCameraView::OnPostRender(GuiRendererContext* ctx)
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
}

//////////////////////////////////////////////////////////////////////////

GameWindow::GameWindow()
    : mCameraEntity(nullptr)
{
}

void GameWindow::InitCamera()
{
    NFE_ASSERT(mScene, "Scene is not created");

    // TODO remove
    // camera entity must be created from Game Object resource
    if (!mCameraEntity)
    {
        mCameraEntity = mScene->GetEntitySystem()->CreateEntity();
        mCameraEntity->SetGlobalPosition(Vector(0.0, 1.75f, -4.0f));

        uint32 width, height;
        GetSize(width, height);

        PerspectiveProjectionDesc perspective;
        perspective.aspectRatio = (float)width / (float)height;
        perspective.farDist = 200.0f;
        perspective.nearDist = 0.05f;
        perspective.FoV = NFE_MATH_PI * 60.0f / 180.0f;

        std::unique_ptr<CameraComponent> camera(new CameraComponent);
        camera->SetPerspective(&perspective);
        mCameraEntity->AddComponent(std::move(camera));
    }

    if (!mView)
    {
        mView.reset(new MainCameraView);
        mView->SetWindow(this);
        mView->SetCamera(mCameraEntity);
    }
}

void GameWindow::SetUpScene(int sceneId, GameWindow* parent)
{
    if (parent == nullptr)  // init a new scene
    {
        SceneManager* newScene = InitScene(sceneId);
        if (newScene == nullptr)
            return;
        mScene.reset(newScene, SceneDeleter);
    }
    else  // fork
    {
        mScene = parent->mScene;
    }

    // TODO
    /*
    GameObject* gameObject = ENGINE_GET_GAME_OBJECT("FreeCamera");
    mCameraInstance = mScene->GetGameObjectSystem()->SpawnGameObject(gameObject);
    */
}

void GameWindow::SpawnTestObject(uint32 objectType, const Vector& position, const Vector& velocity)
{
    NFE_ASSERT(mScene != nullptr, "Scene is not created");

    const char* meshResourceName = nullptr;
    const char* collisionResourceName = nullptr;

    if (objectType == 0) // cube
    {
        meshResourceName = "cube.nfm";
        collisionResourceName = "shape_box";
    }
    else
    {
        LOG_ERROR("Unsupported test object type");
        return;
    }

    Entity* boxEntity = mScene->GetEntitySystem()->CreateEntity();
    boxEntity->SetGlobalPosition(position);

    std::unique_ptr<MeshComponent> mesh(new MeshComponent);
    mesh->SetMeshResource("cube.nfm");
    boxEntity->AddComponent(std::move(mesh));

    std::unique_ptr<BodyComponent> body(new BodyComponent);
    body->SetCollisionShape(ENGINE_GET_COLLISION_SHAPE(collisionResourceName));
    body->SetVelocity(velocity);
    boxEntity->AddComponent(std::move(body));
}

void GameWindow::OnKeyPress(Common::KeyCode key)
{
    // create input event structure for the engine
    Utils::KeyPressedEvent event;
    event.key = key;
    event.isAltPressed = IsKeyPressed(Common::KeyCode::AltLeft);
    event.isCtrlPressed = IsKeyPressed(Common::KeyCode::ControlLeft);
    event.isShiftPressed = IsKeyPressed(Common::KeyCode::ShiftLeft);

    if (mView && mView->OnKeyPressed(event))
        return; // key press event was consumed by the engine

    if (key == Common::KeyCode::F1)
    {
        BOOL fullscreen = GetFullscreenMode();
        SetFullscreenMode(!fullscreen);
    }

    // spawn a new window
    if (key == Common::KeyCode::N)
        AddWindow(this);
}

void GameWindow::OnMouseDown(UINT button, int x, int y)
{
    // create input event structure for the engine
    Utils::MouseButtonEvent event;
    event.mouseButton = button;
    event.x = x;
    event.y = y;

    if (mView && mView->OnMouseDown(event))
        return; // mouse event was consumed by the engine

    if (button == 0)
    {
        const Vector& forwardVector = mCameraEntity->GetGlobalMatrix().GetRow(2);
        SpawnTestObject(0, mCameraEntity->GetGlobalPosition() + forwardVector, Vector());
    }

    // TODO temporary
    if (button == 0)
    {
        mScene->GetInputSystem()->PushEvent(Input::Event(Input::Event::Type::KeyPress, "cameraControl"));
    }
    else if (button == 1)
    {
        mScene->GetInputSystem()->PushEvent(Input::Event(Input::Event::Type::KeyPress, "shoot"));
    }
}

void GameWindow::OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    // create input event structure for the engine
    Utils::MouseMoveEvent event;
    event.x = x;
    event.y = y;

    if (mView && mView->OnMouseMove(event))
        return; // mouse event was consumed by the engine
}

void GameWindow::OnMouseUp(UINT button)
{
    // create input event structure for the engine
    Utils::MouseButtonEvent event;
    event.mouseButton = button;
    GetMousePosition(event.x, event.y);

    if (mView && mView->OnMouseUp(event))
        return; // mouse event was consumed by the engine

    // TODO temporary
    if (button == 0)
    {
        mScene->GetInputSystem()->PushEvent(Input::Event(Input::Event::Type::KeyRelease, "cameraControl"));
    }
    else if (button == 1)
    {
        mScene->GetInputSystem()->PushEvent(Input::Event(Input::Event::Type::KeyRelease, "shoot"));
    }
}

void GameWindow::OnScroll(int delta)
{
    if (mView && mView->OnMouseScroll(delta))
        return; // mouse event was consumed by the engine
}

void GameWindow::OnCharTyped(const char* charUTF8)
{
    if (mView && mView->OnCharTyped(charUTF8))
        return; // mouse event was consumed by the engine
}

// window resized
void GameWindow::OnResize(uint32 width, uint32 height)
{
    if (!mCameraEntity)
    {
        return;
    }

    if (width == 0 || height == 0)
    {
        return;
    }

    CameraComponent* cameraComponent = mCameraEntity->GetComponent<CameraComponent>();
    NFE_ASSERT(cameraComponent, "Invalid camera component");

    PerspectiveProjectionDesc perspective;
    cameraComponent->GetPerspective(&perspective);
    perspective.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    cameraComponent->SetPerspective(&perspective);
}

} // namespace NFE
