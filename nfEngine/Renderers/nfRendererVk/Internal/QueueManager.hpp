#pragma once

#include "Defines.hpp"

#include "nfCommon/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

enum QueueType
{
    General = 0,
    Compute,
    Transfer,
    Count,
};

/**
 * An object managing queues for Device and providing an abstraction layer
 * for multiple queue families.
 *
 * It's main goal is to provide an unified interface for all queue operations
 * while hiding the fact if separate queues are available on the Physical Device.
 * Callee's only need to provide what type of queue is required and the object will
 * return the same, or different queue, depending on the situation.
 */
class QueueManager
{
    friend class Device;

    struct Queue
    {
        VkQueue queue;
        uint32 queueFamilyIndex;
        VkCommandPool commandPool;

        Queue()
            : queue(VK_NULL_HANDLE)
            , queueFamilyIndex(UINT32_MAX)
            , commandPool()
        {
        }
    };

    VkDevice mDeviceRef;

    Queue mQueues[QueueType::Count];

    uint32_t SelectQueueFamily(const Common::DynArray<VkQueueFamilyProperties>& families, VkQueueFlags flag);

public:
    QueueManager();
    ~QueueManager();

    // Initialization is two-stage. First stage prepares QueueInfo structs,
    // second populates mQueues members with data.
    bool Init(VkPhysicalDevice physDevice, bool allowMultipleQueues, Common::DynArray<VkDeviceQueueCreateInfo>& queueInfos);
    bool InitQueues(VkDevice device);
    void Release();

    NFE_INLINE VkQueue GetQueue(QueueType type) const
    {
        return mQueues[type].queue;
    }

    NFE_INLINE uint32 GetQueueFamilyIndex(QueueType type) const
    {
        return mQueues[type].queueFamilyIndex;
    }

    NFE_INLINE VkCommandPool GetCommandPool(QueueType type) const
    {
        return mQueues[type].commandPool;
    }
};

} // namespace Renderer
} // namespace NFE
