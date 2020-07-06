#include "PCH.hpp"
#include "WindowViewport.hpp"
#include "Renderer.hpp"
#include "Engine/Common/Math/Vec4fU.hpp"
#include "../../Renderers/RendererCommon/Fence.hpp"


namespace NFE {
namespace Renderer {

WindowViewport::WindowViewport(Common::Window& window)
    : targetWindow(window)
{
    Renderer& renderer = Renderer::GetInstance();

    uint32 width, height;
    targetWindow.GetSize(width, height);

    {
        BackbufferDesc desc;
        desc.windowHandle = targetWindow.GetHandle();
        desc.commandQueue = renderer.GetGraphicsQueue();
        desc.width = width;
        desc.height = height;
        desc.debugName = "WindowViewportBackbuffer";
        desc.format = Format::B8G8R8A8_U_Norm;

        mBackbuffer = renderer.GetDevice()->CreateBackbuffer(desc);
        NFE_ASSERT(mBackbuffer, "Failed to create backbuffer");
    }

    CreateResources();

    mCommandRecorder = renderer.GetDevice()->CreateCommandRecorder();
}

WindowViewport::~WindowViewport()
{
    Renderer::GetInstance().GetGraphicsQueue()->Signal()->Wait();
}

void WindowViewport::Present()
{
    /*
    {
        mCommandRecorder->Begin(CommandQueueType::Graphics);

        mCommandRecorder->SetRenderTarget(mRenderTarget);

        const Math::Vec4fU color(0.02f, 0.04f, 0.06f, 1.0f);
        mCommandRecorder->Clear(ClearFlagsColor, 1, nullptr, &color);

        mCommandRecorder->CopyTexture(mTexture, mBackbuffer);

        Renderer::GetInstance().GetGraphicsQueue()->Execute(mCommandRecorder->Finish());
    }
    */

    mBackbuffer->Present();
}

void WindowViewport::CreateResources()
{
    uint32 width, height;
    targetWindow.GetSize(width, height);

    {
        TextureDesc desc;
        desc.width = width;
        desc.height = height;
        desc.debugName = "WindowViewportTexture";
        desc.format = Format::B8G8R8A8_U_Norm_sRGB;
        desc.usage = TextureUsageFlag::RenderTarget;
        desc.mode = ResourceAccessMode::GPUOnly;

        mTexture = Renderer::GetInstance().GetDevice()->CreateTexture(desc);
        NFE_ASSERT(mTexture, "Failed to create texture");
    }

    {
        RenderTargetDesc desc;
        desc.targets = { RenderTargetElement(mTexture) };

        mRenderTarget = Renderer::GetInstance().GetDevice()->CreateRenderTarget(desc);
        NFE_ASSERT(mRenderTarget, "Failed to create render target");
    }
}

BackbufferPtr WindowViewport::GetBackbuffer() const
{
    return mBackbuffer;
}

void WindowViewport::Resize(uint32 width, uint32 height)
{
    if (mBackbuffer)
    {
        mBackbuffer->Resize(width, height);
    }

    CreateResources();
}

} // namespace Scene
} // namespace NFE
