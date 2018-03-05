#pragma once

#include "VkRAII.hpp"

namespace NFE {
namespace Renderer {
namespace Tools {

// cannot call this function "CreateSemaphore" because of name collision with WinAPI
VkRAII<VkSemaphore> CreateSem(VkDevice device);

} // namespace Tools
} // namespace Renderer
} // namespace NFE
