#include "PCH.hpp"
#include "GameWindow.hpp"
#include "Main.hpp"
#include "Controllers/FreeCameraController.hpp"

#include "Engine/Core/Scene/Systems/InputSystem.hpp"
#include "Engine/Core/Scene/Components/ComponentLight.hpp"
#include "Engine/Core/Scene/Components/ComponentMesh.hpp"
#include "Engine/Core/Scene/Components/ComponentCamera.hpp"
#include "Engine/Core/Scene/Components/ComponentController.hpp"
#include "Engine/Core/Scene/Components/ComponentTrigger.hpp"
#include "Engine/Core/Utils/SimpleInput.hpp"

#include "Engine/Common/System/Window.hpp"
#include "Engine/Common/System/Assertion.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/System/KeyCodes.hpp"
#include "Engine/Common/Reflection/Types/ReflectionClassType.hpp"

namespace NFE {

using namespace Common;
using namespace Math;
using namespace Resource;


GameWindow::GameWindow()
    : mCameraEntity(nullptr)
{
}

bool GameWindow::InitViewport()
{
    mViewport = MakeUniquePtr<Renderer::WindowViewport>(*this);

    return mViewport != nullptr;
}

void GameWindow::InitCamera()
{
    NFE_ASSERT(mScene, "Scene is not created");

    // TODO remove
    // camera entity must be created from a resource
    {
        mCameraEntity = mScene->GetSystem<Scene::EntitySystem>()->CreateEntity();
        mCameraEntity->SetGlobalPosition(Vec4f(0.0, 1.75f, -4.0f));

        uint32 width, height;
        GetSize(width, height);

        Scene::PerspectiveProjectionDesc perspective;
        perspective.aspectRatio = (float)width / (float)height;
        perspective.farDist = 200.0f;
        perspective.nearDist = 0.05f;
        perspective.FoV = DegToRad(60.0f);

        auto camera = MakeUniquePtr<Scene::CameraComponent>();
        camera->SetPerspective(&perspective);
        mCameraEntity->AddComponent(std::move(camera));

        auto trigger = MakeUniquePtr<Scene::TriggerComponent>();
        trigger->SetSize(Vec4f(0.1f, 0.1f, 0.1f));
        trigger->SetType(Scene::TriggerType::Source);
        mCameraEntity->AddComponent(trigger);

        auto controller = MakeUniquePtr<Scene::ControllerComponent>();
        Scene::EntityControllerPtr controllerObject = StaticCast<Scene::IEntityController>(MakeUniquePtr<FreeCameraController>()); // TODO
        controller->SetController(std::move(controllerObject));
        mCameraEntity->AddComponent(std::move(controller));
    }
}

void GameWindow::SetUpScene(GameWindow* parent)
{
    if (parent == nullptr) // init a new scene
    {
        Scene::ScenePtr newScene = MakeUniquePtr<Scene::Scene>();
        if (newScene == nullptr)
        {
            return;
        }

        if (!newScene->InitializeSystems())
        {
            NFE_LOG_ERROR("Failed to initialize scene's systems");
            return;
        }

        // setup input system
        // TODO this should be in global "input manager"
        Scene::InputSystem* inputSystem = newScene->GetSystem<Scene::InputSystem>();
        if (inputSystem)
        {
            inputSystem->RegisterAxis("MoveForwardBackward", -1.0f, 1.0f, false);
            inputSystem->RegisterAxis("MoveRightLeft", -1.0f, 1.0f, false);
            inputSystem->RegisterAxis("MoveUpDown", -1.0f, 1.0f, false);
            inputSystem->RegisterAxis("MovementSpeed", -1.0f, 1.0f, false);
        }

        mScene = std::move(newScene);
    }
    else  // fork (crate a new window, but use the same scene)
    {
        mScene = parent->mScene;
    }

    InitCamera();
}

void GameWindow::SpawnTestObject(uint32 objectType, const Vec4f& position, const Vec4f& velocity)
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
        NFE_LOG_ERROR("Unsupported test object type");
        return;
    }

    Scene::Entity* boxEntity = mScene->GetSystem<Scene::EntitySystem>()->CreateEntity();
    boxEntity->SetGlobalPosition(position);

    auto mesh = Common::MakeUniquePtr<Scene::MeshComponent>();
    //mesh->SetMeshResource("cube.nfm");
    boxEntity->AddComponent(std::move(mesh));

    //auto body = Common::MakeUniquePtr<Scene::BodyComponent>();
    //body->SetCollisionShape(ENGINE_GET_COLLISION_SHAPE(collisionResourceName));
    //body->SetVelocity(velocity);
    //body->SetMass(10.0f);
    //boxEntity->AddComponent(std::move(body));
}

void GameWindow::OnKeyPress(Common::KeyCode key)
{
    // create input event structure for the engine
    Utils::KeyPressedEvent event;
    event.key = key;
    event.isAltPressed = IsKeyPressed(Common::KeyCode::AltLeft);
    event.isCtrlPressed = IsKeyPressed(Common::KeyCode::ControlLeft);
    event.isShiftPressed = IsKeyPressed(Common::KeyCode::ShiftLeft);

    // TODO this is TEMPORARY !!!
    // there should be some mappings done in the global input manager, like:
    // "W pressed" -> MoveForwardBackward == 1.0f, "S pressed" -> MoveForwardBackward == -1.0f,
    // etc.

    Scene::InputSystem* inputSystem = mScene->GetSystem<Scene::InputSystem>();

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
    else if (key == Common::KeyCode::E)
    {
        inputSystem->PushEvent(Input::EventData(Input::EventData::Type::KeyPress, StringView("Use")));
    }
    else if (key == Common::KeyCode::W)
    {
        inputSystem->SetAxisValue("MoveForwardBackward", 1.0f, true);
    }
    else if (key == Common::KeyCode::S)
    {
        inputSystem->SetAxisValue("MoveForwardBackward", -1.0f, true);
    }
    else if (key == Common::KeyCode::D)
    {
        inputSystem->SetAxisValue("MoveRightLeft", 1.0f, true);
    }
    else if (key == Common::KeyCode::A)
    {
        inputSystem->SetAxisValue("MoveRightLeft", -1.0f, true);
    }
    else if (key == Common::KeyCode::E)
    {
        inputSystem->SetAxisValue("MoveUpDown", 1.0f, true);
    }
    else if (key == Common::KeyCode::Q)
    {
        inputSystem->SetAxisValue("MoveUpDown", -1.0f, true);
    }
    else if (key == Common::KeyCode::ControlLeft)
    {
        inputSystem->SetAxisValue("MovementSpeed", -1.0f, true);
    }
    else if (key == Common::KeyCode::ShiftLeft)
    {
        inputSystem->SetAxisValue("MovementSpeed", 1.0f, true);
    }
}

void GameWindow::OnKeyUp(Common::KeyCode key)
{
    // TODO this is TEMPORARY !!!

    Scene::InputSystem* inputSystem = mScene->GetSystem<Scene::InputSystem>();

    if (key == Common::KeyCode::W)
    {
        inputSystem->SetAxisValue("MoveForwardBackward", -1.0f, true);
    }
    else if (key == Common::KeyCode::S)
    {
        inputSystem->SetAxisValue("MoveForwardBackward", 1.0f, true);
    }
    else if (key == Common::KeyCode::D)
    {
        inputSystem->SetAxisValue("MoveRightLeft", -1.0f, true);
    }
    else if (key == Common::KeyCode::A)
    {
        inputSystem->SetAxisValue("MoveRightLeft", 1.0f, true);
    }
    else if (key == Common::KeyCode::E)
    {
        inputSystem->SetAxisValue("MoveUpDown", -1.0f, true);
    }
    else if (key == Common::KeyCode::Q)
    {
        inputSystem->SetAxisValue("MoveUpDown", 1.0f, true);
    }
    else if (key == Common::KeyCode::ControlLeft)
    {
        inputSystem->SetAxisValue("MovementSpeed", 1.0f, true);
    }
    else if (key == Common::KeyCode::ShiftLeft)
    {
        inputSystem->SetAxisValue("MovementSpeed", -1.0f, true);
    }
}

void GameWindow::OnMouseDown(MouseButton button, int x, int y)
{
    // create input event structure for the engine
    Utils::MouseButtonEvent event;
    event.mouseButton = button;
    event.x = x;
    event.y = y;

    if (button == MouseButton::Left)
    {
        const Vec4f& forwardVector = mCameraEntity->GetGlobalRotation().GetAxisZ();
        SpawnTestObject(0, mCameraEntity->GetGlobalPosition() + forwardVector, Vec4f::Zero());
    }

    // TODO temporary
    Scene::InputSystem* inputSystem = mScene->GetSystem<Scene::InputSystem>();
    if (button == MouseButton::Left)
    {
        inputSystem->PushEvent(Input::EventData(Input::EventData::Type::KeyPress, StringView("Shoot")));
    }
    else if (button == MouseButton::Right)
    {
        inputSystem->PushEvent(Input::EventData(Input::EventData::Type::KeyPress, StringView("CameraControl")));
    }
}

void GameWindow::OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    // create input event structure for the engine
    Utils::MouseMoveEvent event;
    event.x = x;
    event.y = y;

    const float sensitivity = 0.005f;

    // TODO temporary
    Scene::InputSystem* inputSystem = mScene->GetSystem<Scene::InputSystem>();
    if (deltaX != 0)
    {
        const float value = sensitivity * static_cast<float>(deltaX);
        inputSystem->PushEvent(Input::EventData(Input::EventData::Type::Axis, StringView("CameraYaw"), value));
    }
    if (deltaY != 0)
    {
        const float value = sensitivity * static_cast<float>(deltaY);
        inputSystem->PushEvent(Input::EventData(Input::EventData::Type::Axis, StringView("CameraPitch"), value));
    }
}

void GameWindow::OnMouseUp(MouseButton button)
{
    // create input event structure for the engine
    Utils::MouseButtonEvent event;
    event.mouseButton = button;
    GetMousePosition(event.x, event.y);

    // TODO temporary
    Scene::InputSystem* inputSystem = mScene->GetSystem<Scene::InputSystem>();
    if (button == MouseButton::Left)
    {
        inputSystem->PushEvent(Input::EventData(Input::EventData::Type::KeyRelease, StringView("Shoot")));
    }
    else if (button == MouseButton::Right)
    {
        inputSystem->PushEvent(Input::EventData(Input::EventData::Type::KeyRelease, StringView("CameraControl")));
    }
}

void GameWindow::OnScroll(int delta)
{

}

void GameWindow::OnCharTyped(const char* charUTF8)
{

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

    mViewport->Resize(width, height);

    Scene::CameraComponent* cameraComponent = mCameraEntity->GetComponent<Scene::CameraComponent>();
    NFE_ASSERT(cameraComponent, "Invalid camera component");

    Scene::PerspectiveProjectionDesc perspective;
    cameraComponent->GetPerspective(&perspective);
    perspective.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    cameraComponent->SetPerspective(&perspective);

    mCameraEntity->Update();
}

} // namespace NFE
