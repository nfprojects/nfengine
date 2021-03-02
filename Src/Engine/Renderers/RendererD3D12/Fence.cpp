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


FenceData::FenceData()
    : mLastSignaledValue(InitialValue)
    , mLastCompletedValue(InitialValue)
{
}

FenceData::~FenceData()
{
    Release();
}

void FenceData::Release()
{
    if (mFenceObject)
    {
        gDevice->GetFenceManager().UnregisterFenceData(this);

        mFenceObject.Reset();
    }
}

bool FenceData::Init()
{
    HRESULT hr = D3D_CALL_CHECK(gDevice->GetDevice()->CreateFence(InitialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFenceObject.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create D3D12 fence object");
        return false;
    }

    gDevice->GetFenceManager().RegisterFenceData(this);

    return true;
}

uint64 FenceData::GetCompletedValue() const
{
    if (!mFenceObject)
    {
        return InvalidValue;
    }

    return mFenceObject->GetCompletedValue();
}

uint64 FenceData::Signal(ID3D12CommandQueue* queue, const FenceFlags flags, FencePtr* outFencePtr)
{
    NFE_ASSERT(mFenceObject, "Fence object not created");

    FencePtr fence;

    uint64 fenceValue = InvalidValue;
    {
        // force ordering of fence values passed to Signal()
        NFE_SCOPED_LOCK(mLock);

        fenceValue = ++mLastSignaledValue;

        if (outFencePtr)
        {
            fence = MakeSharedPtr<Fence>(fenceValue, flags, mFenceObject.Get());
            *outFencePtr = fence;
        }

        // add to pending list before signaling it
        gDevice->GetFenceManager().OnFenceRequested(this, fenceValue, fence);

        if (FAILED(D3D_CALL_CHECK(queue->Signal(mFenceObject.Get(), fenceValue))))
        {
            NFE_LOG_ERROR("Failed to enqueue fence value update");
            return InvalidValue;
        }
    }
    return fenceValue;
}

void FenceData::OnValueCompleted(uint64 completedValue)
{
    NFE_ASSERT(completedValue <= mLastSignaledValue.load(), "Invalid value");
    NFE_ASSERT(completedValue >= mLastCompletedValue, "Invalid value");

    if (completedValue > mLastCompletedValue)
    {
        mLastCompletedValue = completedValue;

        if (mCallback)
        {
            mCallback(completedValue);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////


Fence::Fence(uint64 fenceValue, const FenceFlags flags, ID3D12Fence* fenceObject)
    : mIsFinished(false)
    , mFenceValue(fenceValue)
    , mFenceObject(fenceObject)
    , mFlags(flags)
{
    TaskDesc desc;
    desc.debugName = "NFE::Renderer::Fence::mDependencyTask";
    desc.priority = ThreadPool::MaxPriority; // max priority so it will unblock other tasks as soon as possible

    mDependencyTask = ThreadPool::GetInstance().CreateTask(desc);
    NFE_ASSERT(mDependencyTask != InvalidTaskID, "Failed to create dependency task");
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


///////////////////////////////////////////////////////////////////////////////////////////////////


FenceManager::FenceManager()
    : mFinish(false)
    , mLoopEvent(INVALID_HANDLE_VALUE)
    , mFenceWaitEvent(INVALID_HANDLE_VALUE)
{
}

FenceManager::~FenceManager()
{
    NFE_ASSERT(mFinish, "Fence manager must be finished");
}

bool FenceManager::Initialize()
{
    mLoopEvent = ::CreateEvent(nullptr, FALSE, FALSE, L"NFE::Renderer::FenceManager::mLoopEvent");
    if (mLoopEvent == INVALID_HANDLE_VALUE)
    {
        NFE_LOG_ERROR("Failed to create event object");
        return false;
    }

    // Create an event handle to use for frame synchronization.
    mFenceWaitEvent = ::CreateEvent(nullptr, FALSE, FALSE, L"NFE::Renderer::FenceData::mWaitEvent");
    if (mFenceWaitEvent == INVALID_HANDLE_VALUE)
    {
        NFE_LOG_ERROR("Failed to create fence event object");
        return false;
    }

    mThread.RunFunction([this]()
    {
        Loop();
    });
    mThread.SetName("NFE::Renderer::FenceManager");
    mThread.SetPriority(ThreadPriority::AboveNormal);

    return true;
}

void FenceManager::Uninitialize()
{
    mFinish = true;
    ::SetEvent(mLoopEvent);

    mThread.Wait();

    if (mLoopEvent != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(mLoopEvent);
        mLoopEvent = INVALID_HANDLE_VALUE;
    }

    if (mFenceWaitEvent != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(mFenceWaitEvent);
        mFenceWaitEvent = INVALID_HANDLE_VALUE;
    }
}

void FenceManager::OnFenceRequested(FenceData* fenceData, uint64 value, const FencePtr& fence)
{
    NFE_SCOPED_LOCK(mPendingFencesLock);

    for (PendingFences& pendingFences : mPendingFences)
    {
        if (pendingFences.fenceData == fenceData && pendingFences.value == value)
        {
            pendingFences.fences.PushBack(fence);
            return;
        }
    }

    // no fence list found for given value, create new entry
    PendingFences newPendingFencesList;
    newPendingFencesList.fenceData = fenceData;
    newPendingFencesList.value = value;
    if (fence)
    {
        newPendingFencesList.fences.PushBack(fence);
    }
    mPendingFences.PushBack(std::move(newPendingFencesList));

    ::SetEvent(mLoopEvent);
}

void FenceManager::RegisterFenceData(const FenceData* fenceData)
{
    NFE_SCOPED_LOCK(mFenceDataLock);

    const auto iter = mFenceData.Find(fenceData);
    NFE_ASSERT(iter == mFenceData.End(), "Fence data already registered");

    mFenceData.PushBack(fenceData);
}

void FenceManager::UnregisterFenceData(const FenceData* fenceData)
{
    NFE_SCOPED_LOCK(mFenceDataLock);

    const auto iter = mFenceData.Find(fenceData);
    NFE_ASSERT(iter != mFenceData.End(), "Fence data not registered");

    mFenceData.Erase(iter);
}

void FenceManager::Loop()
{
    constexpr uint32 MaxFences = 8;
    StaticArray<ID3D12Fence*, MaxFences> fencesToWait;
    StaticArray<uint64, MaxFences> valuesToWait;

    while (!mFinish)
    {
        // build list of events to wait on
        {
            fencesToWait.Clear();
            valuesToWait.Clear();

            NFE_SCOPED_SHARED_LOCK(mFenceDataLock);

            for (const FenceData* fenceData : mFenceData)
            {
                // only wait for pending fences
                if (fenceData->mLastSignaledValue > fenceData->mLastCompletedValue)
                {
                    const uint64 expectedValue = fenceData->mLastCompletedValue + 1;
                    fencesToWait.PushBack(fenceData->mFenceObject.Get());
                    valuesToWait.PushBack(expectedValue);
                }
            }
        }

        if (!fencesToWait.Empty())
        {
            // TODO this may crash if command queue is destroyed in the meantime
            // we should keep fence object reference here
            HRESULT hr = D3D_CALL_CHECK(gDevice->GetDevice()->SetEventOnMultipleFenceCompletion(
                fencesToWait.Data(), valuesToWait.Data(), fencesToWait.Size(), D3D12_MULTIPLE_FENCE_WAIT_FLAG_ANY, mFenceWaitEvent));
            if (FAILED(hr))
            {
                NFE_LOG_ERROR("FenceManager: Failed to setup wait event on %u fences, error code: %u", fencesToWait.Size(), hr);
            }
        }

        uint32 timeout = 10000;

        ::SetLastError(0);
        const HANDLE events[] = { mLoopEvent, mFenceWaitEvent };
        const uint32 numEvents = fencesToWait.Empty() ? 1u : 2u;
        const DWORD waitResult = ::WaitForMultipleObjects(numEvents, events, false, timeout);

        if (waitResult == WAIT_FAILED)
        {
            const DWORD errorCode = ::GetLastError();
            NFE_LOG_ERROR("FenceManager: WaitForMultipleObjects failed, error code: %u", errorCode);
            continue;
        }
        else if (waitResult == WAIT_TIMEOUT)
        {
            NFE_LOG_ERROR("FenceManager: WaitForMultipleObjects timeout");
            continue;
        }

        // flush fences only when mFenceWaitEvent triggered
        if (!fencesToWait.Empty() && waitResult == 1u)
        {
            FlushFinishedFences();
        }
    }

    NFE_LOG_DEBUG("Finishing D3D12 fence manager thread");
}

void FenceManager::FlushFinishedFences()
{
    NFE_SCOPED_LOCK(mPendingFencesLock);

    for (uint32 i = 0; i < mPendingFences.Size(); ++i)
    {
        PendingFences& pendingFencesList = mPendingFences[i];

        const uint64 completedValue = pendingFencesList.fenceData->GetCompletedValue();
        NFE_ASSERT(completedValue >= pendingFencesList.fenceData->mLastCompletedValue, "Invalid completed fence value");

        if (completedValue < pendingFencesList.value)
        {
            // not finished yet
            continue;
        }

        // finished, notify fences
        for (const FenceWeakPtr& fencePtr : pendingFencesList.fences)
        {
            if (const FencePtr fence = fencePtr.Lock())
            {
                Fence* internalFence = static_cast<Fence*>(fence.Get());
                internalFence->OnFenceFinished();
            }
        }

        pendingFencesList.fenceData->OnValueCompleted(completedValue);

        // get rid of list
        mPendingFences.Erase(mPendingFences.Begin() + i);
        i--;
    }
}


} // namespace Renderer
} // namespace NFE
