#include "PCH.hpp"
#include "QueueManager.hpp"

#include "Translations.hpp"
#include "Tools.hpp"

#include "API/Device.hpp"


namespace NFE {
namespace Renderer {

QueueManager::QueueManager()
{
}

QueueManager::~QueueManager()
{
    Release();
}

uint32_t QueueManager::SelectQueueFamily(const Common::DynArray<VkQueueFamilyProperties>& families, VkQueueFlags flag)
{
    if (families.Size() == 0)
        return UINT32_MAX;

    // Goal of this function is to choose a family with lowest possible capabilities,
    // but supporting flag provided in argument. GPUs typically have one uber-family
    // containing all basic capabilities (graphics, compute and transfer), additional
    // families implement only a subset of these caps for specific purposes (ex. device
    // supporting Asynchronous Compute has separate family with only COMPUTE cap set).
    //
    // Choosing queues from separate families could result in performance benefits, so
    // no matter what it's worth to select them.
    uint32 family = UINT32_MAX;
    for (uint32 i = 0; i < families.Size(); ++i)
    {
        if (families[i].queueFlags & flag)
        {
            if ((family == UINT32_MAX) || (families[i].queueFlags < families[family].queueFlags))
                family = i;
        }
    }

    return family;
}

bool QueueManager::Init(VkPhysicalDevice physDevice, bool allowMultipleQueues, Common::DynArray<VkDeviceQueueCreateInfo>& queueInfos)
{
    // Grab queue properties from our selected device
    uint32 queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueCount, nullptr);
    if (queueCount == 0)
    {
        NFE_LOG_ERROR("Physical device does not have any queue family properties.");
        return false;
    }

    Common::DynArray<VkQueueFamilyProperties> queueProps;
    queueProps.Resize(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueCount, queueProps.Data());

    for (uint32 i = 0; i < queueCount; ++i)
    {
        NFE_LOG_DEBUG("Queue #%u:", i);
        NFE_LOG_DEBUG("  Flags: %x (%s)", queueProps[i].queueFlags, TranslateVkQueueFlagsToString(queueProps[i].queueFlags));
    }

    mQueues[QueueType::General].queueFamilyIndex = SelectQueueFamily(queueProps, VK_QUEUE_GRAPHICS_BIT);
    if (mQueues[QueueType::General].queueFamilyIndex == UINT32_MAX)
    {
        NFE_LOG_ERROR("Selected physical device doesn't have a graphics queue, which is required for Vulkan renderer to run.");
        return false;
    }

    mQueues[QueueType::Compute].queueFamilyIndex = SelectQueueFamily(queueProps, VK_QUEUE_COMPUTE_BIT);
    mQueues[QueueType::Transfer].queueFamilyIndex = SelectQueueFamily(queueProps, VK_QUEUE_TRANSFER_BIT);

    float queuePriorities[] = { 0.0f };
    VkDeviceQueueCreateInfo queueInfo;
    VK_ZERO_MEMORY(queueInfo);
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = mQueues[QueueType::General].queueFamilyIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queuePriorities;
    queueInfos.PushBack(queueInfo);

    if (allowMultipleQueues)
    {
        if (mQueues[QueueType::Compute].queueFamilyIndex != mQueues[QueueType::General].queueFamilyIndex)
        {
            queueInfo.queueFamilyIndex = mQueues[QueueType::Compute].queueFamilyIndex;
            queueInfos.PushBack(queueInfo);
        }

        if (mQueues[QueueType::Transfer].queueFamilyIndex != mQueues[QueueType::General].queueFamilyIndex)
        {
            queueInfo.queueFamilyIndex = mQueues[QueueType::Transfer].queueFamilyIndex;
            queueInfos.PushBack(queueInfo);
        }
    }

    return true;
}

bool QueueManager::InitQueues(VkDevice device)
{
    mDeviceRef = device;

    vkGetDeviceQueue(mDeviceRef, mQueues[QueueType::General].queueFamilyIndex, 0, &mQueues[QueueType::General].queue);
    vkGetDeviceQueue(mDeviceRef, mQueues[QueueType::Compute].queueFamilyIndex, 0, &mQueues[QueueType::Compute].queue);
    vkGetDeviceQueue(mDeviceRef, mQueues[QueueType::Transfer].queueFamilyIndex, 0, &mQueues[QueueType::Transfer].queue);

    // General queue should be unique, so it should have its own command pool
    mQueues[QueueType::General].commandPool = Tools::CreateCommandPool(mQueues[QueueType::General].queueFamilyIndex);

    if (mQueues[QueueType::Compute].queueFamilyIndex != mQueues[QueueType::General].queueFamilyIndex)
    {
        // Separate Compute family requires separate Command Pool
        mQueues[QueueType::Compute].commandPool = Tools::CreateCommandPool(mQueues[QueueType::Compute].queueFamilyIndex);
    }
    else
    {
        // Same family as General means we can just copy the handle
        mQueues[QueueType::Compute].commandPool = mQueues[QueueType::General].commandPool;
    }

    // Same as above goes for Transfer queue
    if (mQueues[QueueType::Transfer].queueFamilyIndex != mQueues[QueueType::General].queueFamilyIndex)
    {
        mQueues[QueueType::Transfer].commandPool = Tools::CreateCommandPool(mQueues[QueueType::Transfer].queueFamilyIndex);
    }
    else
    {
        mQueues[QueueType::Transfer].commandPool = mQueues[QueueType::General].commandPool;
    }

    return true;
}

void QueueManager::Release()
{
    if (mQueues[QueueType::General].commandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(mDeviceRef, mQueues[QueueType::General].commandPool, nullptr);

    if (mQueues[QueueType::Compute].queueFamilyIndex != mQueues[QueueType::General].queueFamilyIndex)
    {
        if (mQueues[QueueType::Compute].commandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(mDeviceRef, mQueues[QueueType::Compute].commandPool, nullptr);
    }

    if (mQueues[QueueType::Transfer].queueFamilyIndex != mQueues[QueueType::General].queueFamilyIndex)
    {
        if (mQueues[QueueType::Transfer].commandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(mDeviceRef, mQueues[QueueType::Transfer].commandPool, nullptr);
    }

    mQueues[QueueType::General].queue = VK_NULL_HANDLE;
    mQueues[QueueType::General].queueFamilyIndex = UINT32_MAX;
    mQueues[QueueType::General].commandPool = VK_NULL_HANDLE;

    mQueues[QueueType::Compute].queue = VK_NULL_HANDLE;
    mQueues[QueueType::Compute].queueFamilyIndex = UINT32_MAX;
    mQueues[QueueType::Compute].commandPool = VK_NULL_HANDLE;

    mQueues[QueueType::Transfer].queue = VK_NULL_HANDLE;
    mQueues[QueueType::Transfer].queueFamilyIndex = UINT32_MAX;
    mQueues[QueueType::Transfer].commandPool = VK_NULL_HANDLE;
}

} // namespace Renderer
} // namespace NFE
