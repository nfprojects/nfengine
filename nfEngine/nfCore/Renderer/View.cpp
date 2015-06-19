/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of View class.
 */

#include "../PCH.hpp"
#include "View.hpp"
#include "Renderer.hpp"

#include "../Globals.hpp"
#include "../ResourcesManager.hpp"
#include "../Texture.hpp"

#include "Window.hpp"
#include "Logger.hpp"
#include "Image.hpp"

namespace NFE {
namespace Renderer {

View::View()
{
    camera = nullptr;
    mTexture = nullptr;
}

View::~View()
{
    Release();
}

void View::Release()
{
    if (mTexture != nullptr)
    {
        mTexture->DelRef(this);
        //  mTexture->Unload();
        mTexture = nullptr;
    }
}

void View::OnPostRender(RenderContext* pCtx, GuiRenderer* pGuiRenderer)
{
    // no GUI by default
}

Result View::SetCamera(Scene::Camera* pCamera)
{
    camera = pCamera;
    return Result::OK;
}

Scene::Camera* View::GetCamera() const
{
    return camera;
}


// link the view to a window
Result View::SetWindow(Common::Window* window)
{
    Release();

    uint32 width, height;
    window->GetSize(width, height);

    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = width;
    bbDesc.height = height;
    bbDesc.windowHandle = static_cast<void*>(window->GetHandle());
    bbDesc.vSync = false;
    mWindowBackbuffer.reset(g_pRenderer->GetDevice()->CreateBackbuffer(bbDesc));
    if (mWindowBackbuffer == nullptr)
    {
        LOG_ERROR("Failed to create backbuffer");
        return Result::AllocationError;
    }

    RenderTargetElement rtTarget;
    rtTarget.texture = mWindowBackbuffer.get();

    RenderTargetDesc rtDesc;
    rtDesc.depthBuffer = nullptr; // TODO
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;

    mRenderTarget.reset(g_pRenderer->GetDevice()->CreateRenderTarget(rtDesc));
    if (mRenderTarget == nullptr)
    {
        mWindowBackbuffer.reset();
        LOG_ERROR("Failed to create render target");
        return Result::AllocationError;
    }

    return Result::OK;
}

void View::Present()
{
    if (mWindowBackbuffer != nullptr && mRenderTarget != nullptr)
    {
        mWindowBackbuffer->Present();
    }
}

using namespace Resource;

// create custom, off-screen render target
Texture* View::SetOffScreen(uint32 width, uint32 height, const char* pTextureName)
{
    Release();

    mTexture = (Texture*)g_pResManager->GetResource(pTextureName, ResourceType::Texture);
    if (mTexture == nullptr) return nullptr;

    mTexture->Load();
    mTexture->AddRef(this);

    // TODO
    // mRenderTarget = mTexture->CreateRendertarget(width, height, Common::ImageFormat::RGBA_Float);

    return mTexture;
}

} // namespace Renderer
} // namespace NFE
