#pragma once

#include "VkRAII.hpp"

namespace NFE {
namespace Renderer {
namespace Tools {

VkRAII<VkFence> CreateFence(bool signalled);

// cannot call this function "CreateSemaphore" because of name collision with WinAPI
VkRAII<VkSemaphore> CreateSem();

} // namespace Tools
} // namespace Renderer
} // namespace NFE
