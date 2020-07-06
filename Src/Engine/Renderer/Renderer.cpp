#include "PCH.hpp"
#include "Renderer.hpp"
#include "RenderGraph/RenderGraph.hpp"
#include "RenderGraph/RenderGraphResourceAllocator.hpp"
#include "Viewport/Viewport.hpp"
#include "../../Renderers/RendererCommon/Fence.hpp"

namespace NFE {
namespace Renderer {

using namespace Common;

Renderer::Renderer()
    : mDevice(nullptr)
{
}

Renderer::~Renderer()
{
    NFE_ASSERT(!mDevice, "Render device is still initialized");
}

Renderer& Renderer::GetInstance()
{
    static Renderer rendererInstance;
    return rendererInstance;
}

bool Renderer::Initialize()
{
    if (!mRendererLib.Open("RendererD3D12"))
    {
        return false;
    }

    RendererInitFunc rendererInitFunc;
    if (!mRendererLib.GetSymbol(RENDERER_INIT_FUNC, rendererInitFunc))
    {
        return false;
    }

    DeviceInitParams params;
    params.debugLevel = 1; // TODO
    mDevice = rendererInitFunc(&params);
    if (!mDevice)
    {
        return false;
    }

    mGraphicsQueue = mDevice->CreateCommandQueue(CommandQueueType::Graphics, "Graphics queue");
    mComputeQueue = mDevice->CreateCommandQueue(CommandQueueType::Compute, "Compute queue");
    mCopyQueue = mDevice->CreateCommandQueue(CommandQueueType::Copy, "Copy queue");

    mRenderGraphResourceAllocator = MakeUniquePtr<RenderGraphResourceAllocator>();

    return true;
}

void Renderer::Shutdown()
{
    if (mDevice)
    {
        // wait for queues
        {
            const auto copyQueueFence = mCopyQueue->Signal();
            const auto computeQueueFence = mComputeQueue->Signal();
            const auto graphicsQueueFence = mGraphicsQueue->Signal();

            copyQueueFence->Wait();
            computeQueueFence->Wait();
            graphicsQueueFence->Wait();

            mCopyQueue.Reset();
            mComputeQueue.Reset();
            mGraphicsQueue.Reset();
        }

        mDevice = nullptr;
        RendererReleaseFunc proc;
        if (mRendererLib.GetSymbol(RENDERER_RELEASE_FUNC, proc))
        {
            proc();
        }
    }
}

void Renderer::DrawFrame(const FrameInfo& frameInfo)
{
    RenderGraph graph;

    graph.Build(frameInfo);

    graph.Execute(frameInfo, *mRenderGraphResourceAllocator);
}

} // namespace Scene
} // namespace NFE
