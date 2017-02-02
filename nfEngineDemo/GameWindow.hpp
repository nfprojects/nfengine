#pragma once

#include "nfCommon/Window.hpp"

/**
 * Test game window.
 * Supports forking (after fork the same scene can be viewed from multiple viewports).
 */
class GameWindow : public NFE::Common::Window
{
private:
    std::unique_ptr<MainCameraView> mView;
    std::shared_ptr<Scene::SceneManager> mScene;
    NFE::Scene::GameObjectInstance* mCameraInstance;

public:
    NFE_INLINE Scene::SceneManager* GetScene() const { return mScene.get(); }
    NFE_INLINE Renderer::View* GetView() const { return mView.get(); }
    NFE_INLINE Scene::GameObjectInstance* GetCameraInstance() const { return mCameraInstance; }

    GameWindow();
    void SetUpScene(int sceneId = 0, GameWindow* parent = nullptr);
    void OnKeyPress(Common::KeyCode key) override;
    void OnMouseDown(UINT button, int x, int y) override;
    void OnMouseMove(int x, int y, int deltaX, int deltaY) override;
    void OnMouseUp(UINT button) override;
    void OnScroll(int delta) override;
    void OnCharTyped(const char* charUTF8) override
    void OnResize(uint32 width, uint32 height) override;
};
