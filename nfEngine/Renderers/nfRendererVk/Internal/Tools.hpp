#pragma once

#include "VkHandle.hpp"

namespace NFE {
namespace Renderer {
namespace Tools {

VkHandle<VkCommandPool> CreateCommandPool();
VkHandle<VkFence> CreateFence(bool signalled);

// cannot call this function "CreateSemaphore" because of name collision with WinAPI
VkHandle<VkSemaphore> CreateSem();

} // namespace Tools
} // namespace Renderer
} // namespace NFE
