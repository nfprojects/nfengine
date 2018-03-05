#include "PCH.hpp"
#include "Tools.hpp"
#include "Defines.hpp"

#include "API/Device.hpp"


namespace NFE {
namespace Renderer {
namespace Tools {

VkRAII<VkFence> CreateFence(bool signalled)
{
    VkFenceCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signalled)
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence = VK_NULL_HANDLE;
    VkResult result = vkCreateFence(gDevice->GetDevice(), &info, nullptr, &fence);
    VK_RETURN_EMPTY_VKRAII_IF_FAILED(VkFence, result, "Failed to create Fence");

    return VkRAII<VkFence>(fence, vkDestroyFence);
}

VkRAII<VkSemaphore> CreateSem()
{
    VkSemaphoreCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore sem = VK_NULL_HANDLE;
    VkResult result = vkCreateSemaphore(gDevice->GetDevice(), &info, nullptr, &sem);
    VK_RETURN_EMPTY_VKRAII_IF_FAILED(VkSemaphore, result, "Failed to create Semaphore");

    return VkRAII<VkSemaphore>(sem, vkDestroySemaphore);
}

} // namespace Tools
} // namespace Renderer
} // namespace Tools
