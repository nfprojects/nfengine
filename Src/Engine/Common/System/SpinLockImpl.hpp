/**
 * @file
 * @brief  Spin Lock implementation.
 */

#pragma once

#include "SpinLock.hpp"

namespace NFE {
namespace Common {

bool SpinLock::TryAcquireExclusive()
{
    return !mLocked.test_and_set(std::memory_order_acquire);
}

void SpinLock::AcquireExclusive()
{
    while (mLocked.test_and_set(std::memory_order_acquire))
    { }
}

void SpinLock::ReleaseExclusive()
{
    mLocked.clear(std::memory_order_release);
}

} // namespace Common
} // namespace NFE
