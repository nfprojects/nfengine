#include "PCH.hpp"
#include "GameWindow.hpp"
#include "Scenes.hpp"
#include "Main.hpp"
#include "Controllers/FreeCameraController.hpp"

#include "nfCore/Scene/Systems/InputSystem.hpp"
#include "nfCore/Scene/Components/ComponentBody.hpp"
#include "nfCore/Scene/Components/ComponentLight.hpp"
#include "nfCore/Scene/Components/ComponentMesh.hpp"
#include "nfCore/Scene/Components/ComponentCamera.hpp"
#include "nfCore/Scene/Components/ComponentController.hpp"
#include "nfCore/Scene/Components/ComponentTrigger.hpp"

#include "nfCommon/System/Window.hpp"
#include "nfCommon/System/Assertion.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/System/KeyCodes.hpp"


namespace NFE {

using namespace Common;
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
    {
        mCameraEntity = mScene->GetEntitySystem()->CreateEntity();
        mCameraEntity->SetGlobalPosition(Vector(0.0, 1.75f, -4.0f));

        uint32 width, height;
        GetSize(width, height);

        PerspectiveProjectionDesc perspective;
        perspective.aspectRatio = (float)width / (float)height;
        perspective.farDist = 200.0f;
        perspective.nearDist = 0.05f;
        perspective.FoV = DegToRad(60.0f);

        auto camera = MakeUniquePtr<CameraComponent>();
        camera->SetPerspective(&perspective);
        mCameraEntity->AddComponent(std::move(camera));

        auto body = MakeUniquePtr<BodyComponent>();
        // TODO add some spherical shape so the camera can collide with the scene
        body->SetMass(0.0f);
        mCameraEntity->AddComponent(std::move(body));

        auto controller = MakeUniquePtr<ControllerComponent>();
        EntityControllerPtr controllerObject = StaticCast<IEntityController>(MakeUniquePtr<FreeCameraController>()); // TODO
        controller->SetController(std::move(controllerObject));
        mCameraEntity->AddComponent(std::move(controller));

        auto trigger = MakeUniquePtr<TriggerComponent>();
        trigger->SetSize(Vector(0.1f, 0.1f, 0.1f));
        mCameraEntity->AddComponent(std::move(trigger));
    }

    if (!mView)
    {
        mView.reset(new MainCameraView);
        mView->SetWindow(this);
    }
    mView->SetCamera(mCameraEntity);
}

void GameWindow::SetUpScene(int sceneId, GameWindow* parent)
{
    if (parent == nullptr) // init a new scene
    {
        SceneManager* newScene = InitScene(sceneId);
        if (newScene == nullptr)
        {
            return;
        }

        // setup input system
        // TODO this should be in global "input manager"
        newScene->GetInputSystem()->RegisterAxis("MoveForwardBackward", -1.0f, 1.0f, false);
        newScene->GetInputSystem()->RegisterAxis("MoveRightLeft", -1.0f, 1.0f, false);
        newScene->GetInputSystem()->RegisterAxis("MoveUpDown", -1.0f, 1.0f, false);
        newScene->GetInputSystem()->RegisterAxis("MovementSpeed", -1.0f, 1.0f, false);

        mScene.reset(newScene, SceneDeleter);
    }
    else  // fork (crate a new window, but use the same scene)
    {
        mScene = parent->mScene;
    }

    InitCamera();
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

    UniquePtr<MeshComponent> mesh(new MeshComponent);
    mesh->SetMeshResource("cube.nfm");
    boxEntity->AddComponent(std::move(mesh));

    UniquePtr<BodyComponent> body(new BodyComponent);
    body->SetCollisionShape(ENGINE_GET_COLLISION_SHAPE(collisionResourceName));
    body->SetVelocity(velocity);
    body->SetMass(10.0f);
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
    {
        return; // key press event was consumed by the engine
    }

    // TODO this is TEMPORARY !!!
    // there should be some mappings done in the global input manager, like:
    // "W pressed" -> MoveForwardBackward == 1.0f, "S pressed" -> MoveForwardBackward == -1.0f,
    // etc.

    if (key == Common::KeyCode::F1)
    {
        BOOL fullscreen = GetFullscreenMode();
        SetFullscreenMode(!fullscreen);
    }
    else if (key == Common::KeyCode::N)
    {
        // spawn a new window
        AddWindow(this);
    }
    else if (key >= Common::KeyCode::Num0 && key <= Common::KeyCode::Num9)
    {
        const uint32 sceneNum = static_cast<uint32>(key) - static_cast<uint32>(Common::KeyCode::Num0);
        SetUpScene(sceneNum);
    }
    else if (key == Common::KeyCode::E)
    {
        mScene->GetInputSystem()->PushEvent(Input::EventData(Input::EventData::Type::KeyPress, StringView("Use")));
    }
    else if (key == Common::KeyCode::W)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveForwardBackward", 1.0f, true);
    }
    else if (key == Common::KeyCode::S)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveForwardBackward", -1.0f, true);
    }
    else if (key == Common::KeyCode::D)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveRightLeft", 1.0f, true);
    }
    else if (key == Common::KeyCode::A)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveRightLeft", -1.0f, true);
    }
    else if (key == Common::KeyCode::E)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveUpDown", 1.0f, true);
    }
    else if (key == Common::KeyCode::Q)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveUpDown", -1.0f, true);
    }
    else if (key == Common::KeyCode::ControlLeft)
    {
        mScene->GetInputSystem()->SetAxisValue("MovementSpeed", -1.0f, true);
    }
    else if (key == Common::KeyCode::ShiftLeft)
    {
        mScene->GetInputSystem()->SetAxisValue("MovementSpeed", 1.0f, true);
    }
}

void GameWindow::OnKeyUp(Common::KeyCode key)
{
    // TODO this is TEMPORARY !!!

    if (key == Common::KeyCode::W)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveForwardBackward", -1.0f, true);
    }
    else if (key == Common::KeyCode::S)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveForwardBackward", 1.0f, true);
    }
    else if (key == Common::KeyCode::D)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveRightLeft", -1.0f, true);
    }
    else if (key == Common::KeyCode::A)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveRightLeft", 1.0f, true);
    }
    else if (key == Common::KeyCode::E)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveUpDown", -1.0f, true);
    }
    else if (key == Common::KeyCode::Q)
    {
        mScene->GetInputSystem()->SetAxisValue("MoveUpDown", 1.0f, true);
    }
    else if (key == Common::KeyCode::ControlLeft)
    {
        mScene->GetInputSystem()->SetAxisValue("MovementSpeed", 1.0f, true);
    }
    else if (key == Common::KeyCode::ShiftLeft)
    {
        mScene->GetInputSystem()->SetAxisValue("MovementSpeed", -1.0f, true);
    }
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
        const Vector& forwardVector = mCameraEntity->GetGlobalRotation().Transform(Vector(0.0f, 0.0f, 1.0f));
        SpawnTestObject(0, mCameraEntity->GetGlobalPosition() + forwardVector, Vector());
    }

    // TODO temporary
    if (button == 0)
    {
        mScene->GetInputSystem()->PushEvent(Input::EventData(Input::EventData::Type::KeyPress, StringView("Shoot")));
    }
    else if (button == 1)
    {
        mScene->GetInputSystem()->PushEvent(Input::EventData(Input::EventData::Type::KeyPress, StringView("CameraControl")));
    }
}

void GameWindow::OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    // create input event structure for the engine
    Utils::MouseMoveEvent event;
    event.x = x;
    event.y = y;

    if (mView && mView->OnMouseMove(event))
    {
        // mouse event was consumed by the engine's viewport
        return;
    }

    const float sensitivity = 0.005f;

    // TODO temporary
    if (deltaX != 0)
    {
        const float value = sensitivity * static_cast<float>(deltaX);
        mScene->GetInputSystem()->PushEvent(Input::EventData(Input::EventData::Type::Axis, StringView("CameraYaw"), value));
    }
    if (deltaY != 0)
    {
        const float value = sensitivity * static_cast<float>(deltaY);
        mScene->GetInputSystem()->PushEvent(Input::EventData(Input::EventData::Type::Axis, StringView("CameraPitch"), value));
    }
}

void GameWindow::OnMouseUp(UINT button)
{
    // create input event structure for the engine
    Utils::MouseButtonEvent event;
    event.mouseButton = button;
    GetMousePosition(event.x, event.y);

    if (mView && mView->OnMouseUp(event))
    {
        return; // mouse event was consumed by the engine
    }

    // TODO temporary
    if (button == 0)
    {
        mScene->GetInputSystem()->PushEvent(Input::EventData(Input::EventData::Type::KeyRelease, StringView("Shoot")));
    }
    else if (button == 1)
    {
        mScene->GetInputSystem()->PushEvent(Input::EventData(Input::EventData::Type::KeyRelease, StringView("CameraControl")));
    }
}

void GameWindow::OnScroll(int delta)
{
    if (mView && mView->OnMouseScroll(delta))
    {
        return; // mouse event was consumed by the engine
    }
}

void GameWindow::OnCharTyped(const char* charUTF8)
{
    if (mView && mView->OnCharTyped(charUTF8))
    {
        return; // mouse event was consumed by the engine
    }
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

    mCameraEntity->Update();
}

} // namespace NFE
