#pragma once

#include "Defines.hpp"


namespace NFE {
namespace Renderer {
namespace Tools {


//////////////
// Creators //
//////////////
bool AllocateCommandBuffers(VkCommandPool pool, uint32 count, VkCommandBuffer* buffers);
VkCommandPool CreateCommandPool(uint32 queueFamilyIndex);
VkFence CreateFence(bool signalled);
VkSemaphore CreateSem(); // cannot call this function "CreateSemaphore" because of name collision with WinAPI


////////////////
// Destroyers //
////////////////
void FreeCommandBuffers(VkCommandPool pool, uint32 count, VkCommandBuffer* buffers);
void DestroyCommandPool(VkCommandPool pool);
void DestroyFence(VkFence fence);
void DestroySem(VkSemaphore sem);


} // namespace Tools
} // namespace Renderer
} // namespace NFE
