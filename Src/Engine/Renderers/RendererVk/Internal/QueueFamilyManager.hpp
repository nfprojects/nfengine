#pragma once

#include "Engine/Common/nfCommon.hpp"
#include "Engine/Common/Containers/StaticArray.hpp"
#include "../RendererCommon/CommandQueue.hpp"


namespace NFE {
namespace Renderer {

/**
 * A per-device manager of Queue Families.
 *
 * Initialization is done in two stages:
 * 1. PreInit(VkPhysicalDevice) - gather information from PhysicalDevice regarding Queue properties
 *    and other related data. This also prepares VkDeviceQueueCreateInfo for future device creation
 *    call in Device class. All choices regarding which queues to create and what properties they
 *    have are made at this stage. Calling this makes GetCreateInfos() call return valid data.
 * 2. Init(Device) - Get some post-Device-creation data and initialize further Device-related
 *    objects (like CommandPools). Manager is fully initialized after this call and ready to
 *    provide Queues from requested Families.
 */
class QueueFamilyManager
{
public:
        enum class QueueState
    {
        NotAcquired = 0,
        Available,
        Taken,
    };

    struct Queue
    {
        VkQueue queue;
        QueueState state;

        Queue()
            : queue(VK_NULL_HANDLE)
            , state()
        {
        }
    };

    struct QueueFamily
    {
        CommandQueueType type;
        uint32 familyIndex;
        VkCommandPool commandPool;
        Common::DynArray<Queue> queues;
        Common::DynArray<float> priorities;

        QueueFamily()
            : type(CommandQueueType::Invalid)
            , familyIndex(UINT32_MAX)
            , commandPool(VK_NULL_HANDLE)
            , queues()
            , priorities()
        {
        }
    };

private:
    VkDevice mDevice;
    Common::StaticArray<QueueFamily, static_cast<uint32>(CommandQueueType::Max)> mFamilies;
    Common::StaticArray<VkDeviceQueueCreateInfo, static_cast<uint32>(CommandQueueType::Max)> mCreateInfos;
    static_assert(static_cast<uint32>(CommandQueueType::Max) == 3, "Command queue types has been changed, update this class");

    uint32 DetermineFamilyIndex(const Common::DynArray<VkQueueFamilyProperties> props, CommandQueueType type);

    NFE_FORCE_INLINE uint32 CommandQueueTypeToIndex(CommandQueueType type) const
    {
        switch (type)
        {
        case CommandQueueType::Graphics: return 0;
        case CommandQueueType::Compute: return 1;
        case CommandQueueType::Copy: return 2;
        default:
            NFE_ASSERT(0, "Invalid command queue type");
            return UINT32_MAX;
        }
    }

public:
    QueueFamilyManager();
    ~QueueFamilyManager();

    // Populate initial fields and perform some initial management needed for VkDevice creation
    bool PreInit(VkPhysicalDevice physicalDevice);

    // Further initialize remaining fields and make Manager ready to use. Call right after VkDevice
    // is successfully created.
    bool Init(VkDevice device);

    // Get a Queue from requested Family type. Returns queue and its index in family
    // for freeing purposes
    bool GetQueue(CommandQueueType type, VkQueue& queue, uint32& index);

    // Return Queue to family for other use
    void FreeQueue(CommandQueueType type, uint32 index);

    // Release all resources.
    void Release();

    // True after successfully calling Init()
    NFE_FORCE_INLINE bool IsInitialized()
    {
        return (mDevice != VK_NULL_HANDLE);
    }

    // Valid after successfully calling PreInit()
    NFE_FORCE_INLINE const Common::StaticArray<VkDeviceQueueCreateInfo, static_cast<uint32>(CommandQueueType::Max)>& GetCreateInfos() const
    {
        return mCreateInfos;
    }

    NFE_FORCE_INLINE const QueueFamily& GetQueueFamily(CommandQueueType type) const
    {
        return mFamilies[CommandQueueTypeToIndex(type)];
    }
};

using QueueFamilyManagerPtr = Common::SharedPtr<QueueFamilyManager>;

} // namespace Renderer
} // namespace NFE
