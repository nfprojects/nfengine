#pragma once

#include "RendererApi.hpp"
#include "../Renderers/RendererCommon/Device.hpp"
#include "../Common/System/Library.hpp"

namespace NFE {
namespace Renderer {

struct FrameRenderOptions
{
    // TODO features, quality level, etc.
};

struct FrameInfo
{
    // what scene to render? (can be null)
    Scene* scene = nullptr;

    // where render the frame
    IViewport* viewport = nullptr;

    // what camera to use
    Camera* camera = nullptr;

    FrameRenderOptions options;
};


class NFE_RENDERER_API Renderer
{
public:
    static Renderer& GetInstance();

    bool Initialize();
    void Shutdown();

    // get low-level renderer device
    NFE_FORCE_INLINE IDevice* GetDevice() const { return mDevice; }

    NFE_FORCE_INLINE const CommandQueuePtr& GetGraphicsQueue() const { return mGraphicsQueue; }
    NFE_FORCE_INLINE const CommandQueuePtr& GetComputeQueue() const { return mComputeQueue; }
    NFE_FORCE_INLINE const CommandQueuePtr& GetCopyQueue() const { return mCopyQueue; }

    // TODO list of frame info
    void DrawFrame(const FrameInfo& frameInfo);

private:
    Renderer();
    ~Renderer();

    NFE::Common::Library mRendererLib;
    IDevice* mDevice;

    CommandQueuePtr mGraphicsQueue;
    CommandQueuePtr mComputeQueue;
    CommandQueuePtr mCopyQueue;

    Common::UniquePtr<RenderGraphResourceAllocator> mRenderGraphResourceAllocator;
};


} // namespace Renderer
} // namespace NFE
