/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Thread pool task class declarations.
 */

#include "PCH.hpp"
#include "ThreadPoolTask.hpp"
#include "ThreadPool.hpp"

namespace NFE {
namespace Common {

Task::Task()
{
    Reset();
}

void Task::Reset()
{
    mState = State::Invalid;
    mDependencyState = 0;
    mTasksLeft = 0;
    mParent = InvalidTaskID;
    mDependency = InvalidTaskID;
    mHead = InvalidTaskID;
    mTail = InvalidTaskID;
    mSibling = InvalidTaskID;
    mWaitable = nullptr;
    mDebugName = nullptr;
}

Task::Task(Task&& other)
{
    mState = other.mState.load();
    mTasksLeft = other.mTasksLeft.load();
    mParent = other.mParent;
    mDependency = other.mDependency;
    mHead = other.mHead;
    mTail = other.mTail;
    mSibling = other.mSibling;
    mWaitable = other.mWaitable;
}

} // namespace Common
} // namespace NFE
