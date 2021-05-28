#pragma once

#include "Defines.hpp"
#include "QueueFamilyManager.hpp"
#include "../RendererCommon/Types.hpp"

#include "Engine/Common/Containers/FixedArray.hpp"
#include "Engine/Common/System/RWLock.hpp"


namespace NFE {
namespace Renderer {

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

    VkDevice mDevice;
    CommandQueueType mQueueType;
    VkCommandPool mCommandPool;
    Common::RWLock mLock;
    uint32 mFirstUsed;
    uint32 mCurrentAvailable;
    Common::FixedArray<VkCommandBuffer, VK_COMMAND_BUFFER_POOL_SIZE> mCommandBuffers;
    Common::FixedArray<State, VK_COMMAND_BUFFER_POOL_SIZE> mStates;
    Common::DynArray<VkQueue> mRegisteredQueues;
    Common::DynArray<VkFence> mQueueGuardFences;

public:
    CommandBufferManager();
    ~CommandBufferManager();

    bool Init(VkDevice device, const QueueFamilyManager& qfm, CommandQueueType queueType);
    VkCommandBuffer Acquire();
    void Free(VkCommandBuffer buffer);
    void FinishFrame();
    bool RegisterQueue(VkQueue queue);
    bool UnregisterQueue(VkQueue queue);
    void Release();
};

} // namespace Renderer
} // namespace NFE
