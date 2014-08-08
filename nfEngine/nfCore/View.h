/**
    NFEngine project

    \file   View.h
    \brief  Declarations of View class.
*/

#pragma once

#include "Core.h"
#include "Renderer.h"

namespace NFE {
namespace Render {

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
    virtual void OnPostRender(IRenderContext* pCtx, IGuiRenderer* pGuiRenderer);

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

} // namespace Render
} // namespace NFE
