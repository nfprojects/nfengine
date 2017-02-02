#pragma once

#include "nfCore/Renderer/View.hpp"

#include "nfCommon/System/Window.hpp"

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

    // TODO remove
    Scene::Entity* mCameraEntity;

public:
    NFE_INLINE Scene::SceneManager* GetScene() const { return mScene.get(); }
    NFE_INLINE Renderer::View* GetView() const { return mView.get(); }

    GameWindow();

    // Window virtual methods implementation
    virtual void OnKeyPress(Common::KeyCode key) override;
    virtual void OnMouseDown(UINT button, int x, int y) override;
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY) override;
    virtual void OnMouseUp(UINT button) override;
    virtual void OnScroll(int delta) override;
    virtual void OnCharTyped(const char* charUTF8) override;
    virtual void OnResize(uint32 width, uint32 height) override;

    // Create camera game object instance
    void InitCamera();

    // Initialize with a new scene
    // TODO pass scene as resource path
    void SetUpScene(int sceneId = 0, GameWindow* parent = nullptr);

    // TODO temporary - remove
    // spawn test object (cube, barrel, etc.)
    void SpawnTestObject(uint32 objectType, const Math::Vector& position, const Math::Vector& velocity);
};

} // namespace NFE
