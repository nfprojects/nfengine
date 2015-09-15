/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of View class.
 */

#pragma once

#include "../Core.hpp"
#include "HighLevelRenderer.hpp"
#include "RendererResources.hpp"
#include "../EntityManager.hpp"

namespace NFE {
namespace Renderer {

class CORE_API View
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

    Scene::SceneManager* mScene;
    Scene::EntityID mCameraEntity;

    bool InitRenderTarget(ITexture* texture, uint32 width, uint32 height);
    static void OnWindowResize(void* userData);

public:
    ViewSettings settings;
    bool drawAntTweakBar;

    View();
    virtual ~View();

    /**
     * @brief Virtual function called after 3D and post-process pass. Could be used to draw GUI, HUD, etc.
     */
    virtual void OnPostRender(RenderContext* context);

    bool SetCamera(Scene::SceneManager* scene, Scene::EntityID cameraEntity);

    NFE_INLINE Scene::SceneManager* GetSceneManager() const
    {
        return mScene;
    }

    NFE_INLINE Scene::EntityID GetCameraEntity() const
    {
        return mCameraEntity;
    }

    NFE_INLINE IRenderTarget* GetRenderTarget() const
    {
        return mRenderTarget.get();
    }

    NFE_INLINE GeometryBuffer* GetGeometryBuffer() const
    {
        return mGBuffer.get();
    }

    /**
     * @brief Link the view to a window
     * @return Result::OK on success
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
     * Display the render target on a screen, when the View is connected with a window.
     */
    void Present();

    /**
     * Get view dimensions.
     */
    void GetSize(uint32& width, uint32& height);
};

} // namespace Renderer
} // namespace NFE
