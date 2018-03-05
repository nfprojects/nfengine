/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   VkSemaphore Manager implementation
 */

#include "PCH.hpp"
#include "SemaphorePool.hpp"
#include "Device.hpp"

namespace NFE {
namespace Renderer {

SemaphorePool::SemaphorePool(VkDevice device)
    : mDeviceRef(device)
    , mPreviousSemaphore(0)
    , mCurrentSemaphore(0)
{
}

SemaphorePool::~SemaphorePool()
{
    // no need to wait - Device destructor will wait for GPU for us
    for (auto& sem : mSemaphores)
    {
        if (sem != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(mDeviceRef, sem, nullptr);
        }
    }
}

bool SemaphorePool::Init(uint32 semaphoreCount)
{
    mSemaphores.Resize(semaphoreCount);

    VkSemaphoreCreateInfo semInfo;
    VK_ZERO_MEMORY(semInfo);
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result;
    for (auto& sem : mSemaphores)
    {
        result = vkCreateSemaphore(mDeviceRef, &semInfo, nullptr, &sem);
        VK_RETURN_FALSE_IF_FAILED(result, "Failed to create semaphore");
    }

    mCurrentSemaphore = 0;
    mPreviousSemaphore = semaphoreCount - 1;

    NFE_LOG_DEBUG("Created %d semaphores in pool", semaphoreCount);
    return true;
}

void SemaphorePool::Advance()
{
    mPreviousSemaphore = mCurrentSemaphore;

    ++mCurrentSemaphore;
    if (mCurrentSemaphore >= mSemaphores.Size())
    {
        mCurrentSemaphore = 0;
    }
}

} // namespace Renderer
} // namespace NFE
