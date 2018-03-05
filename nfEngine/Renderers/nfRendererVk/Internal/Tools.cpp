#include "PCH.hpp"
#include "Tools.hpp"
#include "Defines.hpp"

#include "API/Device.hpp"


namespace NFE {
namespace Renderer {
namespace Tools {

VkHandle<VkCommandPool> CreateCommandPool(uint32 queueFamilyIndex)
{
    VkCommandPoolCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool pool = VK_NULL_HANDLE;
    VkResult result = vkCreateCommandPool(gDevice->GetDevice(), &info, nullptr, &pool);
    VK_RETURN_EMPTY_VKHANDLE_IF_FAILED(VkCommandPool, result, "Failed to create Command Pool for queue family #%d", queueFamilyIndex);

    return VkHandle<VkCommandPool>(pool, vkDestroyCommandPool);
}

VkHandle<VkFence> CreateFence(bool signalled)
{
    VkFenceCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signalled)
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence = VK_NULL_HANDLE;
    VkResult result = vkCreateFence(gDevice->GetDevice(), &info, nullptr, &fence);
    VK_RETURN_EMPTY_VKHANDLE_IF_FAILED(VkFence, result, "Failed to create Fence");

    return VkHandle<VkFence>(fence, vkDestroyFence);
}

VkHandle<VkSemaphore> CreateSem()
{
    VkSemaphoreCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore sem = VK_NULL_HANDLE;
    VkResult result = vkCreateSemaphore(gDevice->GetDevice(), &info, nullptr, &sem);
    VK_RETURN_EMPTY_VKHANDLE_IF_FAILED(VkSemaphore, result, "Failed to create Semaphore");

    return VkHandle<VkSemaphore>(sem, vkDestroySemaphore);
}

} // namespace Tools
} // namespace Renderer
} // namespace Tools
