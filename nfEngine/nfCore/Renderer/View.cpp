/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of View class.
 */

#include "../PCH.hpp"
#include "View.hpp"
#include "HighLevelRenderer.hpp"

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
    mCamera = nullptr;
    mTexture = nullptr;
    mWindow = nullptr;
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

    if (mWindow != nullptr)
    {
        mWindow->SetResizeCallback(nullptr, nullptr);
        mWindow = nullptr;
    }
}

void View::OnPostRender(RenderContext* context)
{
    // no GUI by default
}

Result View::SetCamera(Scene::Camera* camera)
{
    this->mCamera = camera;
    return Result::OK;
}

Scene::Camera* View::GetCamera() const
{
    return mCamera;
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
    mWindowBackbuffer.reset(gRenderer->GetDevice()->CreateBackbuffer(bbDesc));
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

    mRenderTarget.reset(gRenderer->GetDevice()->CreateRenderTarget(rtDesc));
    if (mRenderTarget == nullptr)
    {
        mWindowBackbuffer.reset();
        LOG_ERROR("Failed to create render target");
        return Result::AllocationError;
    }

    window->SetResizeCallback(OnWindowResize, this);
    mWindow = window;

    return Result::OK;
}

void View::OnWindowResize(void* userData)
{
    View* view = static_cast<View*>(userData);

    if (view->mWindowBackbuffer != nullptr && view->mWindow != nullptr)
    {
        /// make sure that the backbuffer is not used
        ICommandBuffer* commandBuffer = gRenderer->GetImmediateContext()->commandBuffer;
        commandBuffer->SetRenderTarget(nullptr);
        view->mRenderTarget.reset();

        uint32 width, height;
        view->mWindow->GetSize(width, height);
        view->mWindowBackbuffer->Resize(width, height);


        RenderTargetElement rtTarget;
        rtTarget.texture = view->mWindowBackbuffer.get();

        RenderTargetDesc rtDesc;
        rtDesc.depthBuffer = nullptr; // TODO
        rtDesc.numTargets = 1;
        rtDesc.targets = &rtTarget;

        /// create new rendertarget
        view->mRenderTarget.reset(gRenderer->GetDevice()->CreateRenderTarget(rtDesc));
        if (view->mRenderTarget == nullptr)
        {
            view->mWindowBackbuffer.reset();
            LOG_ERROR("Failed to create render target");
        }
    }
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
Texture* View::SetOffScreen(uint32 width, uint32 height, const char* textureName)
{
    Release();

    mTexture = (Texture*)g_pResManager->GetResource(textureName, ResourceType::Texture);
    if (mTexture == nullptr) return nullptr;

    mTexture->Load();
    mTexture->AddRef(this);

    // TODO
    // mRenderTarget = mTexture->CreateRendertarget(width, height, Common::ImageFormat::RGBA_Float);

    return mTexture;
}

void View::GetSize(uint32& width, uint32& height)
{
    if (mWindow != nullptr)
    {
        mWindow->GetSize(width, height);
    }

    // TODO: get off-screen view dimensions when it's implemented
}

} // namespace Renderer
} // namespace NFE
