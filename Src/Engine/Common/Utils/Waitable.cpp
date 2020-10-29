#include "PCH.hpp"
#include "Waitable.hpp"
#include "../System/Thread.hpp"

namespace NFE {
namespace Common {

Waitable::Waitable()
    : mFinished(false)
{ }

Waitable::~Waitable()
{
    Wait();
}

void Waitable::Wait()
{
    NFE_ASSERT(Thread::IsMainThread(), "Nothing should wait on non-main thread as it may cause deadlock");

    if (!mFinished)
    {
        ScopedExclusiveLock<Mutex> lock(mMutex);
        while (!mFinished)
        {
            mConditionVariable.Wait(lock);
        }
    }
}

void Waitable::OnFinished()
{
    const bool oldState = mFinished.exchange(true);
    NFE_ASSERT(!oldState, "OnFinished can be called only once on waitable object");

    ScopedExclusiveLock<Mutex> lock(mMutex);
    mConditionVariable.SignalAll();

}

} // namespace Common
} // namespace NFE
