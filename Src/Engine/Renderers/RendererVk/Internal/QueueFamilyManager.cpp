#include "PCH.hpp"
#include "QueueFamilyManager.hpp"
#include "Defines.hpp"

#include "Internal/Utilities.hpp"
#include "Internal/Debugger.hpp"


namespace NFE {
namespace Renderer {

QueueFamilyManager::QueueFamilyManager()
    : mDevice(VK_NULL_HANDLE)
    , mFamilies()
{
}

QueueFamilyManager::~QueueFamilyManager()
{
    Release();
}

uint32 QueueFamilyManager::DetermineFamilyIndex(const Common::DynArray<VkQueueFamilyProperties> props, CommandQueueType type)
{
    // It is in general more preferable to choose less-capable queue from more-capable one; for
    // example a Device can have following families:
    //    * GRAPHICS | COMPUTE | TRANSFER
    //    * COMPUTE
    //    * TRANSFER
    //
    // To help GPU driver parallelize it would be best in this case to submit Compute tasks onto
    // a queue from COMPUTE family rather than one from GRAPHICS | COMPUTE | TRANSFER family.

    uint32 familyIndex = UINT32_MAX;
    VkQueueFlags lastCaps = VK_QUEUE_FLAG_BITS_MAX_ENUM;
    VkQueueFlags vkQueueType = Util::CommandQueueTypeToVkQueueFlags(type);

    for (uint32 i = 0; i < props.Size(); ++i)
    {
        if (((props[i].queueFlags & vkQueueType) == vkQueueType) &&
            (lastCaps > props[i].queueFlags))
        {
            familyIndex = i;
            lastCaps = props[i].queueFlags;
        }
    }

    return familyIndex;
}

bool QueueFamilyManager::PreInit(VkPhysicalDevice physicalDevice)
{
    uint32 familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    if (familyCount == 0)
    {
        NFE_LOG_ERROR("Physical device does not have any queue family properties.");
        return false;
    }

    Common::DynArray<VkQueueFamilyProperties> familyProps;
    familyProps.Resize(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, familyProps.Data());

    NFE_LOG_DEBUG("Physical Device has %u available queue families:", familyCount);
    for (uint32 i = 0; i < familyCount; ++i)
    {
        NFE_LOG_DEBUG("Queue #%u:", i);
        NFE_LOG_DEBUG("  Flags: %x", familyProps[i].queueFlags);
        NFE_LOG_DEBUG("  Count: %u", familyProps[i].queueCount);
    }

    mFamilies.Resize(familyCount);
    mCreateInfos.Resize(familyCount);

    for (uint32 i = 0; i < mFamilies.Size(); ++i)
    {
        mFamilies[i].type = static_cast<CommandQueueType>(i);
        mFamilies[i].familyIndex = DetermineFamilyIndex(familyProps, mFamilies[i].type);
        mFamilies[i].queues.Resize(familyProps[mFamilies[i].familyIndex].queueCount);
        mFamilies[i].priorities.Resize(mFamilies[i].queues.Size());
        // all priorities for the queues are equal
        for (uint32 p = 0; p < mFamilies[i].queues.Size(); ++p)
            mFamilies[i].priorities[p] = 0.5f;

        // Get as many queues as possible from each
        VK_ZERO_MEMORY(mCreateInfos[i]);
        mCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        mCreateInfos[i].queueFamilyIndex = mFamilies[i].familyIndex;
        mCreateInfos[i].queueCount = mFamilies[i].queues.Size();
        mCreateInfos[i].pQueuePriorities = mFamilies[i].priorities.Data();
    }

    return true;
}

bool QueueFamilyManager::Init(VkDevice device)
{
    mDevice = device;

    VkResult result = VK_SUCCESS;
    Common::String poolNamePrefix("CommandPool-");

    VkCommandPoolCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    for (uint32 i = 0; i < mFamilies.Size(); ++i)
    {
        info.queueFamilyIndex = mFamilies[i].familyIndex;
        result = vkCreateCommandPool(mDevice, &info, nullptr, &mFamilies[i].commandPool);
        CHECK_VKRESULT(result, "Failed to create Command Pool");

        if (Debugger::Instance().IsDebugAnnotationActive())
        {
            Common::String poolName = poolNamePrefix + CommandQueueTypeToStr(mFamilies[i].type);
            Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mFamilies[i].commandPool),
                                            VK_OBJECT_TYPE_COMMAND_POOL,
                                            poolName.Str());
        }
    }

    return true;
}

bool QueueFamilyManager::GetQueue(CommandQueueType type, VkQueue& queue, uint32& index)
{
    uint32 familyIdx = CommandQueueTypeToIndex(type);
    QueueFamily& family = mFamilies[familyIdx];
    bool found = false;

    for (uint32 i = 0; i < family.queues.Size(); ++i)
    {
        if (family.queues[i].state == QueueState::Taken)
            continue;

        if (family.queues[i].state == QueueState::NotAcquired)
        {
            VkDeviceQueueInfo2 info;
            VK_ZERO_MEMORY(info);
            info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
            info.queueFamilyIndex = family.familyIndex;
            info.queueIndex = i;
            vkGetDeviceQueue2(mDevice, &info, &family.queues[i].queue);
        }

        family.queues[i].state = QueueState::Taken;
        queue = family.queues[i].queue;
        index = i;
        found = true;
        break;
    }

    if (!found)
        NFE_LOG_ERROR("Cannot get %s Queue - family exhausted (max %d)", CommandQueueTypeToStr(type), family.queues.Size());

    return found;
}

void QueueFamilyManager::FreeQueue(CommandQueueType type, uint32 index)
{
    Queue& q = mFamilies[CommandQueueTypeToIndex(type)].queues[index];
    NFE_ASSERT(q.state != QueueState::Available, "Freeing already freed queue");
    q.state = QueueState::Available;
}

void QueueFamilyManager::Release()
{
    for (uint32 i = 0; i < mFamilies.Size(); ++i)
    {
        if (mFamilies[i].commandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(mDevice, mFamilies[i].commandPool, nullptr);
    }

    mDevice = VK_NULL_HANDLE;
}


} // namespace Renderer
} // namespace NFE
