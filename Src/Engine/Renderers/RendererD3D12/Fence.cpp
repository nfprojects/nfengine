/**
 * @file
 * @author  Witek902
 * @brief   D3D12 implementation of renderer's fence
 */

#include "PCH.hpp"
#include "Fence.hpp"
#include "RendererD3D12.hpp"
#include "CommandListManager.hpp"

#include "Engine/Common/Utils/ScopedLock.hpp"
#include "Engine/Common/Utils/ThreadPool.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;


FenceData::FenceData()
    : mLastSignaledValue(InitialValue)
    , mLastCompletedValue(InitialValue)
    , mWaitEvent(INVALID_HANDLE_VALUE)
    , mFenceManager(nullptr)
{
}

FenceData::~FenceData()
{
    Release();
}

void FenceData::Release()
{
    mFenceObject.Reset();

    if (mWaitEvent != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(mWaitEvent);
        mWaitEvent = INVALID_HANDLE_VALUE;
    }
}

bool FenceData::Init(FenceManager* fenceManager, Device* device)
{
    mFenceManager = fenceManager;
    mFenceManager->RegisterFenceData(this);

    HRESULT hr = D3D_CALL_CHECK(device->GetDevice()->CreateFence(InitialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFenceObject.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create D3D12 fence object");
        return false;
    }

    // Create an event handle to use for frame synchronization.
    mWaitEvent = ::CreateEvent(nullptr, FALSE, FALSE, L"NFE::Renderer::FenceData::mWaitEvent");
    if (mWaitEvent == INVALID_HANDLE_VALUE)
    {
        NFE_LOG_ERROR("Failed to create fence event object");
        return false;
    }

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

uint64 FenceData::SignalValue(ID3D12CommandQueue* queue)
{
    NFE_ASSERT(mFenceObject, "Fence object not created");

    FencePtr fence;

    uint64 fenceValue;
    {
        // force ordering of fence values passed to Signal()
        NFE_SCOPED_LOCK(mLock);

        fenceValue = ++mLastSignaledValue;

        if (FAILED(D3D_CALL_CHECK(queue->Signal(mFenceObject.Get(), fenceValue))))
        {
            NFE_LOG_ERROR("Failed to enqueue fence value update");
            fenceValue = InvalidValue;
        }
    }

    return fenceValue;
}

FencePtr FenceData::Signal(ID3D12CommandQueue* queue)
{
    NFE_ASSERT(mFenceObject, "Fence object not created");

    FencePtr fence;

    uint64 fenceValue = InvalidValue;
    {
        // force ordering of fence values passed to Signal()
        NFE_SCOPED_LOCK(mLock);

        fenceValue = ++mLastSignaledValue;

        fence = MakeSharedPtr<Fence>(fenceValue);

        // add to pending list before signaling it
        mFenceManager->OnFenceRequested(this, fenceValue, fence);

        if (FAILED(D3D_CALL_CHECK(queue->Signal(mFenceObject.Get(), fenceValue))))
        {
            NFE_LOG_ERROR("Failed to enqueue fence value update");
            return nullptr;
        }
    }

    return fence;
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


Fence::Fence(uint64 fenceValue)
    : mIsFinished(false)
    , mFenceValue(fenceValue)
{
    TaskDesc desc;
    desc.debugName = "NFE::Renderer::Fence::mDependencyTask";
    desc.priority = ThreadPool::MaxPriority; // max priority so it will unblock other tasks as soon as possible

    mDependencyTask = ThreadPool::GetInstance().CreateTask(desc);
    NFE_ASSERT(mDependencyTask != InvalidTaskID, "Failed to create dependency task");
}

Fence::~Fence()
{
    NFE_ASSERT(mIsFinished.load(), "Fence should be finished when destroying. Otherwise it may create deadlock as the dependency won't be fulfilled");
}

bool Fence::IsFinished() const
{
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
{
}

FenceManager::~FenceManager()
{
    mFinish = true;
}

void FenceManager::Initialize()
{
    mLoopEvent = ::CreateEvent(nullptr, FALSE, FALSE, L"NFE::Renderer::FenceManager::mLoopEvent");
    if (mLoopEvent == INVALID_HANDLE_VALUE)
    {
        return;
    }

    mThread.RunFunction([this]()
    {
        Loop();
    });
    mThread.SetName("NFE::Renderer::FenceManager");
}

void FenceManager::Uninitialize()
{
    mFinish = true;
    ::SetEvent(mLoopEvent);

    mThread.Wait();
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
    newPendingFencesList.fences.PushBack(fence);
    mPendingFences.PushBack(std::move(newPendingFencesList));

    ::SetEvent(mLoopEvent);
}

void FenceManager::RegisterFenceData(const FenceData* fenceData)
{
    mFenceData.PushBack(fenceData);
}

void FenceManager::Loop()
{
    constexpr uint32 MaxEvents = 8;
    static_assert(MaxEvents <= MAXIMUM_WAIT_OBJECTS, "Too many events");
    StaticArray<HANDLE, MaxEvents> events;

    while (!mFinish)
    {
        // build list of events to wait on
        {
            events.Clear();

            events.PushBack(mLoopEvent);

            for (const FenceData* fenceData : mFenceData)
            {
                HANDLE handle = fenceData->mWaitEvent;
                uint64 expectedValue = fenceData->mLastSignaledValue;
                fenceData->mFenceObject->SetEventOnCompletion(expectedValue, handle);
                events.PushBack(handle);
            }
        }

        uint32 timeout = 1000;

        ::SetLastError(0);
        const DWORD waitResult = ::WaitForMultipleObjects(events.Size(), events.Data(), false, timeout);

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

        FlushFinishedFences();
    }
}

void FenceManager::FlushFinishedFences()
{
    NFE_SCOPED_LOCK(mPendingFencesLock);

    // TODO this is a bit lame...
    // could at least use event index returned from WaitForMultipleObjects to narrow the search

    for (uint32 i = 0; i < mPendingFences.Size(); ++i)
    {
        PendingFences& pendingFencesList = mPendingFences[i];

        const uint64 completedValue = pendingFencesList.fenceData->GetCompletedValue();

        if (completedValue < pendingFencesList.value)
        {
            // not finished yet
            continue;
        }

        pendingFencesList.fenceData->OnValueCompleted(completedValue);

        // finished, notify fences
        for (const FenceWeakPtr& fencePtr : pendingFencesList.fences)
        {
            if (const FencePtr fence = fencePtr.Lock())
            {
                Fence* internalFence = static_cast<Fence*>(fence.Get());
                internalFence->OnFenceFinished();
            }
        }

        // get rid of list
        mPendingFences.Erase(mPendingFences.Begin() + i);
        i--;
    }
}


} // namespace Renderer
} // namespace NFE
