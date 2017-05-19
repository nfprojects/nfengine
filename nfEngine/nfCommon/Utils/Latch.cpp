/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Latch methods definition.
 */

#include "PCH.hpp"
#include "Latch.hpp"
#include "ScopedLock.hpp"
#include "../System/Timer.hpp"


namespace NFE {
namespace Common {

Latch::Latch()
    : mSet(false)
{
}

void Latch::Set()
{
    ScopedMutexLock lock(mMutex);
    mSet = true;
    mCV.SignalAll();
}

void Latch::Wait()
{
    ScopedMutexLock lock(mMutex);
    while (!mSet)
    {
        mCV.Wait(lock);
    }
}

bool Latch::Wait(const unsigned int timeoutMs)
{
    Timer timer;
    timer.Start();

    const double timeoutSeconds = static_cast<double>(timeoutMs) / 1000.0f;

    ScopedMutexLock lock(mMutex);
    while (!mSet)
    {
        if (timer.Stop() >= timeoutSeconds)
        {
            return false;
        }

        if (!mCV.WaitFor(lock, timeoutMs))
        {
            return false;
        }
    }

    return true;
}

} // namespace Common
} // namespace NFE
