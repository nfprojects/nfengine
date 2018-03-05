#include "PCH.hpp"
#include "QueueManager.hpp"

#include "Translations.hpp"



namespace NFE {
namespace Renderer {

QueueManager::QueueManager()
{
}

QueueManager::~QueueManager()
{
}

uint32_t QueueManager::SelectQueueFamily(const Common::DynArray<VkQueueFamilyProperties>& families, VkQueueFlags flag)
{
    // Goal of this function is to choose a family with lowest possible capabilities,
    // but supporting flag provided in argument. GPUs typically have one uber-family
    // containing all basic capabilities (graphics, compute and transfer). Additional
    // families implement only a subset of these caps for specific purposes (ex. device
    // supporting Asynchronous Compute has separate family with only COMPUTE cap set).
    //
    // Choosing queues from separate families could result in performance benefits, so
    // no matter what it's worth to select them.
    uint32 family = UINT32_MAX;
    for (uint32 i = 0; i < families.Size(); ++i)
    {
        if ((families[i].queueFlags & flag) && (families[i].queueFlags < families[family].queueFlags))
        {
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

    mQueues[QueueType::General].queueFamilyIndex = SelectQueueFamily(queueProps, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
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
            queueInfo.queueFamilyIndex = mQueues[QueueType::General].queueFamilyIndex;
            queueInfos.PushBack(queueInfo);
        }

        if (mQueues[QueueType::Transfer].queueFamilyIndex != mQueues[QueueType::General].queueFamilyIndex)
        {
            queueInfo.queueFamilyIndex = mQueues[QueueType::General].queueFamilyIndex;
            queueInfos.PushBack(queueInfo);
        }
    }

    return true;
}

bool QueueManager::InitQueues(VkDevice device)
{
    vkGetDeviceQueue(device, mQueues[QueueType::General].queueFamilyIndex, 0, &mQueues[QueueType::General].queue);
    vkGetDeviceQueue(device, mQueues[QueueType::Compute].queueFamilyIndex, 0, &mQueues[QueueType::Compute].queue);
    vkGetDeviceQueue(device, mQueues[QueueType::Transfer].queueFamilyIndex, 0, &mQueues[QueueType::Transfer].queue);


}

} // namespace Renderer
} // namespace NFE
