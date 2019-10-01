#pragma once

#include "nfCore/Scene/Scene.hpp"
#include "nfCommon/System/Window.hpp"
#include "nfCommon/Containers/UniquePtr.hpp"
#include "nfCommon/Containers/SharedPtr.hpp"


namespace NFE {

/**
 * Test game window.
 * Supports forking (after fork the same scene can be viewed from multiple viewports).
 */
class GameWindow : public Common::Window
{
public:
    GameWindow();

    Scene::Scene* GetScene() const { return mScene.Get(); }

    // Window virtual methods implementation
    virtual void OnKeyPress(Common::KeyCode key) override;
    virtual void OnKeyUp(Common::KeyCode key) override;
    virtual void OnMouseDown(Common::MouseButton button, int x, int y) override;
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY) override;
    virtual void OnMouseUp(Common::MouseButton button) override;
    virtual void OnScroll(int delta) override;
    virtual void OnCharTyped(const char* charUTF8) override;
    virtual void OnResize(uint32 width, uint32 height) override;

    // Create camera game object instance
    void InitCamera();

    // Initialize with a new scene
    // TODO pass scene as resource path
    void SetUpScene(GameWindow* parent = nullptr);

    // TODO temporary - remove
    // spawn test object (cube, barrel, etc.)
    void SpawnTestObject(uint32 objectType, const Math::Vector4& position, const Math::Vector4& velocity);

private:
    Common::SharedPtr<Scene::Scene> mScene;

    // TODO remove
    Scene::Entity* mCameraEntity;
};

} // namespace NFE
