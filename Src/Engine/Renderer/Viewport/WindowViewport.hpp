#pragma once

#include "Viewport.hpp"
#include "Engine/Common/System/Window.hpp"
#include "../../Renderers/RendererCommon/Backbuffer.hpp"
#include "../../Renderers/RendererCommon/Texture.hpp"
#include "../../Renderers/RendererCommon/RenderTarget.hpp"

namespace NFE {
namespace Renderer {

struct WindowViewportOptions
{
    uint32 width;
    uint32 height;
    uint32 refreshRate;
    bool fullscreen;
    bool vsync;
};

class NFE_RENDERER_API WindowViewport : public IViewport
{
public:
    WindowViewport(Common::Window& window);
    ~WindowViewport();

    virtual void Present() override;
    virtual BackbufferPtr GetBackbuffer() const override;
    virtual void Resize(uint32 width, uint32 height) override;

    virtual uint32 GetWidth() const override { return mOptions.width; }
    virtual uint32 GetHeight() const override { return mOptions.height; }

private:
    Common::Window& targetWindow;
    WindowViewportOptions mOptions;
    BackbufferPtr mBackbuffer;

    void CreateResources();

    // TODO temporary, remove
    TexturePtr mTexture;
    RenderTargetPtr mRenderTarget;
    CommandRecorderPtr mCommandRecorder;
};

} // namespace Renderer
} // namespace NFE
