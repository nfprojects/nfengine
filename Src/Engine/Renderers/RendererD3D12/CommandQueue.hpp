/**
 * @file
 * @author  Witek902
 * @brief   Declaration of Direct3D 12 render's command queue object
 */

#pragma once

#include "Fence.hpp"

#include "../RendererCommon/CommandQueue.hpp"
#include "../../Common/System/RWLock.hpp"


namespace NFE {
namespace Renderer {

class Device;

enum class CommandQueueFamily
{
    GraphicsCompute,
    Copy,
    Invalid,
};

class CommandQueue : public ICommandQueue
{
public:
    CommandQueue();
    ~CommandQueue();

    bool Init(CommandQueueType type);

    NFE_FORCE_INLINE ID3D12CommandQueue* GetQueue() const { return mQueue.Get(); }

    virtual CommandQueueType GetType() const override { return mType; }

    virtual void Submit(
        const Common::ArrayView<ICommandList*> commandLists,
        const Common::ArrayView<IFence*> waitFences) override;

    virtual FencePtr Signal() override;

private:
    Common::RWLock mLock;
    D3DPtr<ID3D12CommandQueue> mQueue;
    FenceData mFenceData;
    CommandQueueType mType;
};

CommandQueueFamily GetCommandQueueFamily(CommandQueueType type);
D3D12_COMMAND_LIST_TYPE TranslateCommandListType(CommandQueueType type);

} // namespace Renderer
} // namespace NFE
