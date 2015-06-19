/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of View class.
 */

#pragma once

#include "../Core.hpp"
#include "HighLevelRenderer.hpp"

namespace NFE {
namespace Renderer {

class CORE_API View
{
    // not NULL when rendering to a off-screen render target
    Resource::Texture* mTexture;  

    // not NULL when rendering to a window
    std::unique_ptr<IBackbuffer> mWindowBackbuffer;

    std::unique_ptr<IRenderTarget> mRenderTarget;
    Scene::Camera* mCamera;

public:

    ViewSettings settings;

    View();
    virtual ~View();

    /**
     * @brief Virtual function called after 3D and post-process pass. Could be used to draw GUI, HUD, etc.
     */
    virtual void OnPostRender(RenderContext* context);

    Result SetCamera(Scene::Camera* camera);
    Scene::Camera* GetCamera() const;

    NFE_INLINE IRenderTarget* GetRenderTarget() const
    {
        return mRenderTarget.get();
    }

    /**
     * @brief Link the view to a window
     * @return Result::OK on success
     */
    Result SetWindow(Common::Window* window);

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
};

} // namespace Renderer
} // namespace NFE
