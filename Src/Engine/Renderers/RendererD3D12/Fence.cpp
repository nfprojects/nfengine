/**
 * @file
 * @author  Witek902
 * @brief   D3D12 implementation of renderer's fence
 */

#include "PCH.hpp"
#include "Fence.hpp"
#include "RendererD3D12.hpp"
#include "CommandListManager.hpp"
#include "Device.hpp"

#include "Engine/Common/Utils/ScopedLock.hpp"
#include "Engine/Common/Utils/ThreadPool.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;

Fence::Fence(uint64 fenceValue, const FenceFlags flags, ID3D12Fence* fenceObject)
    : mIsFinished(false)
    , mFenceValue(fenceValue)
    , mFenceObject(fenceObject)
    , mFlags(flags)
    , mDependencyTask(InvalidTaskID)
{
    if (mFlags & FenceFlag_CpuWaitable)
    {
        TaskDesc desc;
        desc.debugName = "NFE::Renderer::Fence::mDependencyTask";
        desc.priority = ThreadPool::MaxPriority; // max priority so it will unblock other tasks as soon as possible

        mDependencyTask = ThreadPool::GetInstance().CreateTask(desc);
        NFE_ASSERT(mDependencyTask != InvalidTaskID, "Failed to create dependency task");
    }
}

Fence::~Fence()
{
    if (mFlags & FenceFlag_CpuWaitable)
    {
        NFE_ASSERT(mIsFinished.load(), "Fence should be finished when destroying. Otherwise it may create deadlock as the dependency won't be fulfilled");
    }
}

bool Fence::IsFinished() const
{
    NFE_ASSERT(mFlags & FenceFlag_CpuWaitable, "Fence is not CPU-waitable");

    return mIsFinished;
}

void Fence::OnFenceFinished()
{
    NFE_ASSERT(mFlags & FenceFlag_CpuWaitable, "Fence is not CPU-waitable");

    NFE_SCOPED_LOCK(mLock);

    const bool wasFinished = mIsFinished.exchange(true);
    NFE_ASSERT(!wasFinished, "Fence was already finished. This should happen only once");

    // dispatch task so other dependent tasks will kick off
    ThreadPool::GetInstance().DispatchTask(mDependencyTask);
}

void Fence::Sync(TaskBuilder& taskBuilder)
{
    NFE_ASSERT(mFlags & FenceFlag_CpuWaitable, "Fence is not CPU-waitable");

    NFE_SCOPED_LOCK(mLock);

    if (!mIsFinished) // don't put unnecessary dependency in the task builder
    {
        // inject prepared dependency task
        taskBuilder.CustomTask(mDependencyTask);
    }
}

} // namespace Renderer
} // namespace NFE
