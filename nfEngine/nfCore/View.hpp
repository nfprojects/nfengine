/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of View class.
 */

#pragma once

#include "Core.hpp"
#include "Renderer.hpp"

namespace NFE {
namespace Renderer {

class CORE_API View
{
    Resource::Texture* mTexture;  // != NULL when rendering to a off-screen render target

public:
    // TEMPORARY!!!
    IRenderTarget* RT;   // render target
    Scene::Camera* camera;       // camera (optional)

    ViewSettings settings;

    View();
    virtual ~View();

    /**
     * @breif Virtual function called after 3D and post-process pass. Could be used to draw GUI, HUD, etc.
     */
    virtual void OnPostRender(RenderContext* pCtx, GuiRenderer* pGuiRenderer);

    Result SetCamera(Scene::Camera* pCamera);
    Scene::Camera* GetCamera() const;

    /**
     * @breif Link the view to a window
     * @return Result::OK on success
     */
    Result SetWindow(Common::Window* pWindow);

    /**
     * @breif Create custom, off-screen render target
     * @param pTextureName Name of a texture resource associated with the view
     * @return Pointer to the created texture object
     */
    // TODO: more parameters (pixel format, etc.)
    Resource::Texture* SetOffScreen(uint32 width, uint32 height, const char* pTextureName);

    // destroy render target
    void Release();
};

} // namespace Renderer
} // namespace NFE
