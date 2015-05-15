/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Latch methods definition.
 */

#include "PCH.hpp"
#include "Latch.hpp"

#include <chrono>

namespace NFE {
namespace Common {

Latch::Latch()
    : mSet(false)
{
}

void Latch::Set()
{
    Lock lock(mMutex);
    mSet = true;
    mCV.notify_all();
}

void Latch::Wait()
{
    Lock lock(mMutex);
    mCV.wait(lock, [this] { return mSet.load(); });
}

bool Latch::Wait(const unsigned int timeoutMs)
{
    Lock lock(mMutex);
    return mCV.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                        [this] { return mSet.load(); });
}

} // namespace Common
} // namespace NFE
