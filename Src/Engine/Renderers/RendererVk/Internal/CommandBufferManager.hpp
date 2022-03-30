#pragma once

#include "Defines.hpp"
#include "QueueFamilyManager.hpp"
#include "../RendererCommon/Types.hpp"

#include "Engine/Common/Containers/FixedArray.hpp"
#include "Engine/Common/System/RWLock.hpp"

#include <functional>


namespace NFE {
namespace Renderer {

using OnFreeCallback = std::function<void()>;
void DefaultOnFree();


class CommandBufferManager
{
    enum class State
    {
        Invalid = 0,
        Available,
        Acquired,
        AwaitingFinish,
        AwaitingFree,
    };

    struct CommandBufferData
    {
        State state;
        OnFreeCallback onFree;

        CommandBufferData()
            : state(State::Invalid)
            , onFree(DefaultOnFree)
        {}
    };

    VkDevice mDevice;
    CommandQueueType mQueueType;
    VkCommandPool mCommandPool;
    Common::RWLock mLock;
    uint32 mFirstUsed;
    uint32 mCurrentAvailable;
    Common::FixedArray<VkCommandBuffer, VK_COMMAND_BUFFER_POOL_SIZE> mCommandBuffers;
    Common::FixedArray<CommandBufferData, VK_COMMAND_BUFFER_POOL_SIZE> mCBData;
    Common::DynArray<VkQueue> mRegisteredQueues;
    Common::DynArray<VkFence> mQueueGuardFences;

public:
    CommandBufferManager();
    ~CommandBufferManager();

    bool Init(VkDevice device, const QueueFamilyManager& qfm, CommandQueueType queueType);
    VkCommandBuffer Acquire();
    void Free(VkCommandBuffer buffer, OnFreeCallback onFree = DefaultOnFree);
    void FinishFrame();
    bool RegisterQueue(VkQueue queue);
    bool UnregisterQueue(VkQueue queue);
    void Release();
};

} // namespace Renderer
} // namespace NFE
