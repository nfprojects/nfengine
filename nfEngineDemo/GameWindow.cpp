#include "PCH.hpp"
#include "GameWindow.hpp"
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


GameWindow::GameWindow()
    : mScene(nullptr)
{
}

void GameWindow::SetUpScene(int sceneId = 0, CustomWindow* parent = nullptr)
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

    // TODO temporary
    GameObject* gameObject = ENGINE_GET_GAME_OBJECT("FreeCamera");
    mCameraInstance = mScene->GetGameObjectSystem()->SpawnGameObject(gameObject);
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
void GameWindow::OnResize(UINT width, UINT height)
{
    if (!mCameraGameObject)
    {
        return;
    }

    if (width == 0 || height == 0)
    {
        return;
    }

    Entity* cameraEntity = mCameraGameObject->GetEntity();
    NFE_ASSERT(cameraEntity, "Invalid camera entity");

    CameraComponent* cameraComponent = cameraEntity->GetComponent<CameraComponent>();
    NFE_ASSERT(cameraComponent, "Invalid camera component");

    PerspectiveProjectionDesc perspective;
    cameraComponent->GetPerspective(&perspective);
    perspective.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    cameraComponent->SetPerspective(&perspective);
}
