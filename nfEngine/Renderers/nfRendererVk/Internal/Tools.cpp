#include "PCH.hpp"
#include "Tools.hpp"

#include "API/Device.hpp"


namespace NFE {
namespace Renderer {

extern Common::SharedPtr<Device> gDevice;

namespace Tools {

bool AllocateCommandBuffers(VkCommandPool pool, uint32 count, VkCommandBuffer* buffers)
{
    VkCommandBufferAllocateInfo cbInfo;
    VK_ZERO_MEMORY(cbInfo);
    cbInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbInfo.commandPool = pool;
    cbInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbInfo.commandBufferCount = count;
    VkResult result = vkAllocateCommandBuffers(gDevice->GetDevice(), &cbInfo, buffers);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to allocate %d Vulkan Command Buffers", count);

    return true;
}

VkCommandPool CreateCommandPool(uint32 queueFamilyIndex)
{
    VkCommandPoolCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool pool = VK_NULL_HANDLE;
    VkResult result = vkCreateCommandPool(gDevice->GetDevice(), &info, nullptr, &pool);
    VK_RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Command Pool for queue family #%d", queueFamilyIndex);

    return pool;
}

VkFence CreateFence(bool signalled)
{
    VkFenceCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signalled)
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence = VK_NULL_HANDLE;
    VkResult result = vkCreateFence(gDevice->GetDevice(), &info, nullptr, &fence);
    VK_RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Fence");

    return fence;
}

VkSemaphore CreateSem()
{
    VkSemaphoreCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore sem = VK_NULL_HANDLE;
    VkResult result = vkCreateSemaphore(gDevice->GetDevice(), &info, nullptr, &sem);
    VK_RETURN_NULL_HANDLE_IF_FAILED(result, "Failed to create Semaphore");

    return sem;
}

void FreeCommandBuffers(VkCommandPool pool, uint32 count, VkCommandBuffer* buffers)
{
    vkFreeCommandBuffers(gDevice->GetDevice(), pool, count, buffers);
}

void DestroyCommandPool(VkCommandPool pool)
{
    if (pool != VK_NULL_HANDLE)
        vkDestroyCommandPool(gDevice->GetDevice(), pool, nullptr);
}

void DestroyFence(VkFence fence)
{
    if (fence != VK_NULL_HANDLE)
        vkDestroyFence(gDevice->GetDevice(), fence, nullptr);
}

void DestroySem(VkSemaphore sem)
{
    if (sem != VK_NULL_HANDLE)
        vkDestroySemaphore(gDevice->GetDevice(), sem, nullptr);
}

} // namespace Tools
} // namespace Renderer
} // namespace Tools
