/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   VkSemaphore Manager implementation
 */

#include "PCH.hpp"
#include "SemaphorePool.hpp"
#include "Device.hpp"
#include "Tools.hpp"

namespace NFE {
namespace Renderer {

SemaphorePool::SemaphorePool(VkDevice device)
    : mDeviceRef(device)
    , mPreviousSemaphore(0)
    , mCurrentSemaphore(0)
{
}

bool SemaphorePool::Init(uint32 semaphoreCount)
{
    for (uint32_t i = 0; i < semaphoreCount; ++i)
    {
        mSemaphores.EmplaceBack(Tools::CreateSem());
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
