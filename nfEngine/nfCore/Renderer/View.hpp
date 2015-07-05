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
    Resource::Texture* mTexture;  // != NULL when rendering to a off-screen render target

public:
    // TEMPORARY!!!
    IRenderTarget* RT;   // render target
    Scene::Camera* camera;       // camera (optional)

    ViewSettings settings;

    View();
    virtual ~View();

    /**
     * @brief Virtual function called after 3D and post-process pass. Could be used to draw GUI, HUD, etc.
     */
    virtual void OnPostRender(RenderContext* context);

    Result SetCamera(Scene::Camera* camera);
    Scene::Camera* GetCamera() const;

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

    // destroy render target
    void Release();
};

} // namespace Renderer
} // namespace NFE
