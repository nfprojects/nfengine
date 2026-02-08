/**
 * @file
 * @author  Witek902
 * @brief   Declaration of Direct3D 12 render's command queue object
 */

#pragma once

#include "Fence.hpp"

#include "../RendererCommon/CommandQueue.hpp"
#include "../../Common/System/Mutex.hpp"
#include "../../Common/System/Thread.hpp"


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

    bool Init(CommandQueueType type, const char* debugName);

    NFE_FORCE_INLINE ID3D12CommandQueue* GetQueue() const { return mQueue.Get(); }

    virtual CommandQueueType GetType() const override { return mType; }

    virtual void Submit(
        const Common::ArrayView<ICommandList*> commandLists,
        const Common::ArrayView<IFence*> waitFences) override;

    virtual FencePtr Signal(const FenceFlags flags = FenceFlag_CpuWaitable) override;

private:

    static constexpr uint64_t InitialFenceValue = 1;

    void OnFenceCompleted(uint64_t fenceValue);

    void FenceThreadFunc();

    Common::Mutex mLock;
    D3DPtr<ID3D12CommandQueue> mQueue;
    CommandQueueType mType;

    // list of pending objects and fence value associated with them
    struct PendingData
    {
        uint64_t fenceValue = 0;
        Common::WeakPtr<IFence> fencePtr;
    };
    Common::DynArray<PendingData> mPendingData;

    D3DPtr<ID3D12Fence> mFence;
    uint64_t mFenceValue = InitialFenceValue;
    Common::Thread mThread;
    std::atomic<bool> mFinishThread = false;
};

CommandQueueFamily GetCommandQueueFamily(CommandQueueType type);
D3D12_COMMAND_LIST_TYPE TranslateCommandListType(CommandQueueType type);

} // namespace Renderer
} // namespace NFE
