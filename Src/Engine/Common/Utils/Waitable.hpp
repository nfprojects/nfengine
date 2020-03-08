#pragma once

#include "../nfCommon.hpp"
#include "../System/Mutex.hpp"
#include "../System/ConditionVariable.hpp"

#include <atomic>

namespace NFE {
namespace Common {

/**
 * Helper class allowing for waiting for an event.
 */
class NFCOMMON_API Waitable final
{
    NFE_MAKE_NONCOPYABLE(Waitable);
    NFE_MAKE_NONMOVEABLE(Waitable);

public:
    Waitable();
    ~Waitable();

    // Check if the task has been finished.
    NFE_INLINE bool IsFinished() { return mFinished.load(); }

    // Wait for a task to finish.
    // NOTE This can be called only on the main thread!
    void Wait();

    void OnFinished();

private:

    Mutex mMutex;
    ConditionVariable mConditionVariable;
    std::atomic<bool> mFinished;
};



} // namespace Common
} // namespace NFE
