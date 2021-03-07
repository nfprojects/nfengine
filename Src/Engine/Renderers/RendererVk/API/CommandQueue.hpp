#pragma once

#include "../RendererCommon/CommandQueue.hpp"
#include "Internal/QueueFamilyManager.hpp"


namespace NFE {
namespace Renderer {

class CommandQueue: public ICommandQueue
{
    VkQueue mQueue;
    CommandQueueType mType;
    uint32 mIndex;

public:
    CommandQueue();
    ~CommandQueue();

    bool Init(CommandQueueType type, const char* debugName);
    void Release();

    CommandQueueType GetType() const override;

    void Submit(const Common::ArrayView<ICommandList*> commandLists,
                const Common::ArrayView<IFence*> waitFences) override;
    FencePtr Signal(const FenceFlags flags) override;
};

} // namespace Renderer
} // namespace NFE
