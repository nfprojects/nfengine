#pragma once

#include "nfCore/Renderer/View.hpp"

#include "nfCommon/Window.hpp"

#include <memory>

namespace NFE {

class MainCameraView : public Renderer::View
{
public:
    bool showViewProperties;

    MainCameraView();
    virtual void OnDrawImGui(void* state) override;
    virtual void OnPostRender(Renderer::GuiRendererContext* ctx) override;
};

/**
 * Test game window.
 * Supports forking (after fork the same scene can be viewed from multiple viewports).
 */
class GameWindow : public Common::Window
{
private:
    std::unique_ptr<MainCameraView> mView;
    std::shared_ptr<Scene::SceneManager> mScene;
    Scene::GameObjectInstance* mCameraInstance;

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
    void OnCharTyped(const char* charUTF8) override;
    void OnResize(uint32 width, uint32 height) override;
};

} // namespace NFE
