#pragma once

#include "../RendererCommon/CommandQueue.hpp"


namespace NFE {
namespace Renderer {

class CommandQueue: public ICommandQueue
{

public:
    CommandQueue();
    ~CommandQueue();

    bool Init(CommandQueueType type, const char* debugName);

    CommandQueueType GetType() const override;

    void Submit(const Common::ArrayView<ICommandList*> commandLists,
                const Common::ArrayView<IFence*> waitFences) override;
    FencePtr Signal(const FenceFlags flags) override;
};

} // namespace Renderer
} // namespace NFE
