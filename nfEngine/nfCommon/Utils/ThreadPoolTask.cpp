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
    mTasksLeft = 0;
    mParent = NFE_INVALID_TASK_ID;
    mInstancesNum = 0;
    mNextInstance = 0;
    mInstancesLeft = 0;
    mDependency = NFE_INVALID_TASK_ID;
    mHead = NFE_INVALID_TASK_ID;
    mTail = NFE_INVALID_TASK_ID;
    mSibling = NFE_INVALID_TASK_ID;
    mWaitable = false;
}

Task::Task(Task&& other)
{
    mState = other.mState.load();
    mTasksLeft = other.mTasksLeft.load();
    mParent = other.mParent;
    mInstancesNum = other.mInstancesNum;
    mNextInstance = other.mNextInstance;
    mInstancesLeft = other.mInstancesLeft.load();
    mDependency = other.mDependency;
    mHead = other.mHead;
    mTail = other.mTail;
    mSibling = other.mSibling;
    mWaitable = other.mWaitable;
}

} // namespace Common
} // namespace NFE
