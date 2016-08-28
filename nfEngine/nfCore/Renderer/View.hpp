/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of View class.
 */

#pragma once

#include "../Core.hpp"
#include "HighLevelRenderer.hpp"
#include "RendererResources.hpp"
#include "ImGuiWrapper.hpp"
#include "../Scene/EntityManager.hpp"
#include "../Utils/SimpleInput.hpp"

namespace NFE {

// predeclarations
namespace Utils
{
class DebugConsole;
}

namespace Renderer {

struct PostProcessParameters
{
    bool enabled;
    float saturation;
    float noiseFactor;
    float exposureOffset;

    PostProcessParameters()
        : enabled(true)
        , saturation(1.0f)
        , noiseFactor(0.015f)
        , exposureOffset(0.0f)
    {}
};

class CORE_API View : public Utils::SimpleInputListener
{
    // not NULL when rendering to a off-screen render target
    Resource::Texture* mTexture;

    // not NULL when rendering to a window
    std::unique_ptr<IBackbuffer> mWindowBackbuffer;
    Common::Window* mWindow;

    /// TODO: depth buffers, g-buffers and rendertargets should be managed
    /// by the HighLevelRenderer. For example, multiple views can have the same dimensions,
    /// so keeping separate copies will be a waste of RAM.
    std::unique_ptr<GeometryBuffer> mGBuffer;
    std::unique_ptr<IRenderTarget> mRenderTarget;

    std::unique_ptr<ITexture> mTemporaryBuffer;
    std::unique_ptr<IResourceBindingInstance> mTemporaryBufferPostprocessBinding;
    std::unique_ptr<IRenderTarget> mTemporaryRenderTarget;  // before postprocess

    Scene::SceneManager* mScene;
    Scene::EntityID mCameraEntity;

    std::unique_ptr<ImGuiWrapper> mImGuiWrapper;
    std::unique_ptr<Utils::DebugConsole> mDebugConsole;

    // input listeners attached to this view (e.g. ImGui, debug console, etc.)
    std::vector<Utils::SimpleInputListener*> mInputListeners;

    bool InitTemporaryRenderTarget(uint32 width, uint32 height);
    bool InitRenderTarget(ITexture* texture, uint32 width, uint32 height);
    static void OnWindowResize(void* userData);

public:
    PostProcessParameters postProcessParams;

    View();
    virtual ~View();

    /**
     * @brief Virtual function called after 3D and post-process pass. Could be used to draw GUI, HUD, etc.
     */
    virtual void OnPostRender(RenderContext* context);

    /**
     * Virtual function called when a custom ImGui widgets can be drawn.
     * @param state ImGui internal state that must be used as a parameter
                    of use ImGui::SetInternalState function in order to set valid ImGui state
                    (which is hold in global variable).
     */
    virtual void OnDrawImGui(void* state);

    void RegisterInputListener(Utils::SimpleInputListener* listener);
    void UnregisterInputListener(Utils::SimpleInputListener* listener);

    // SimpleInputListener
    bool OnKeyPressed(const Utils::KeyPressedEvent& event) override;
    bool OnMouseDown(const Utils::MouseButtonEvent& event) override;
    bool OnMouseUp(const Utils::MouseButtonEvent& event) override;
    bool OnMouseMove(const Utils::MouseMoveEvent& event) override;
    bool OnMouseScroll(int delta) override;
    bool OnCharTyped(const char* charUTF8) override;

    bool SetCamera(Scene::SceneManager* scene, Scene::EntityID cameraEntity);

    NFE_INLINE Scene::SceneManager* GetSceneManager() const
    {
        return mScene;
    }

    NFE_INLINE Scene::EntityID GetCameraEntity() const
    {
        return mCameraEntity;
    }

    /**
     * Get render target.
     * @param afterPostProcess If set to "true", it returns render target that must be used for
     *                         rendering after post-process pass. For example: debug info, GUI.
     * @return Render target interface pointer
     */
    NFE_INLINE IRenderTarget* GetRenderTarget(bool afterPostProcess = false) const
    {
        if (afterPostProcess)
            return mRenderTarget.get();

        if (postProcessParams.enabled && mTemporaryRenderTarget)
            return mTemporaryRenderTarget.get();

        return mRenderTarget.get();
    }

    NFE_INLINE GeometryBuffer* GetGeometryBuffer() const
    {
        return mGBuffer.get();
    }

    /**
     * @brief Link the view to a window
     * @return true on success
     */
    bool SetWindow(Common::Window* window);

    /**
     * @brief Create custom, off-screen render target
     * @param width,height
     * @param textureName Name of a texture resource associated with the view
     * @return Pointer to the created texture object
     */
    // TODO: more parameters (pixel format, etc.)
    Resource::Texture* SetOffScreen(uint32 width, uint32 height, const char* textureName);

    /**
     * Destroy render target
     */
    void Release();

    /**
     * Perform post-process. If something more needs to be rendered after postprocessing,
     * pass "true" to @p GetRenderTarget method after this call.
     * @param ctx Rendering context
     */
    void Postprocess(RenderContext* ctx);

    /**
     * Display the render target on a screen, when the View is connected with a window.
     */
    void Present();

    /**
     * Get view dimensions.
     */
    void GetSize(uint32& width, uint32& height);

    /**
     * Draw GUI overlay.
     */
    void DrawGui(RenderContext* context);

    /**
     * Draw ImGui window with the view properties.
     */
    void DrawViewPropertiesGui();
};

} // namespace Renderer
} // namespace NFE
