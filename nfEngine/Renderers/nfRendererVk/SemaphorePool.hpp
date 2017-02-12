/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   VkSemaphore Manager class declaration
 */

#pragma once

#include "Defines.hpp"


namespace NFE {
namespace Renderer {

/**
 * A helper class implementing a queue of VkSemaphore objects.
 *
 * To submit commands to GPU, Vulkan requires two VkSemaphore objects. One of these is waited upon
 * before execution, second is signalled right after execution ends. To avoid dynamic VkSemaphore
 * allocation, SemaphoreManager class will supply various parts of the Renderer with semaphores
 * to use during submission.
 */
class SemaphorePool
{
    VkDevice mDeviceRef;

    std::vector<VkSemaphore> mSemaphores;
    uint32 mPreviousSemaphore;
    uint32 mCurrentSemaphore;

public:
    SemaphorePool(VkDevice device);
    ~SemaphorePool();

    bool Init(uint32 semaphoreCount);
    void Advance();

    NFE_INLINE VkSemaphore GetPreviousSemaphore() const
    {
        return mSemaphores[mPreviousSemaphore];
    }

    NFE_INLINE VkSemaphore GetCurrentSemaphore() const
    {
        return mSemaphores[mCurrentSemaphore];
    }
};

} // namespace Renderer
} // namespace NFE
