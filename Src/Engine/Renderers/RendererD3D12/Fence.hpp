/**
 * @file
 * @author  Witek902
 * @brief   Declaration of Direct3D 12 render's fence object
 */

#pragma once

#include "../RendererCommon/Fence.hpp"
#include "Common.hpp"

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/System/RWLock.hpp"
#include "Engine/Common/System/Thread.hpp"


namespace NFE {
namespace Renderer {

class Device;
class FenceManager;

class Fence : public IFence
{
public:
    Fence(uint64 fenceValue, ID3D12Fence* fenceObject);
    ~Fence();

    NFE_FORCE_INLINE uint64 GetValue() const { return mFenceValue; }
    NFE_FORCE_INLINE ID3D12Fence* GetD3DFence() const { return mFenceObject; }

    virtual bool IsFinished() const override;
    virtual void Sync(Common::TaskBuilder& taskBuilder) override;

    void OnFenceFinished();

private:
    ID3D12Fence* mFenceObject; // TODO command queue - SharedPtr/WeakPtr
    uint64 mFenceValue;
    Common::TaskID mDependencyTask;
    std::atomic<bool> mIsFinished;
    Common::RWLock mLock;
};


// ID3D12Fence wrapper
class FenceData
{
    friend class FenceManager;

public:
    static constexpr uint64 InitialValue = 0;
    static constexpr uint64 InvalidValue = UINT64_MAX;

    using Callback = std::function<void(uint64)>;

    FenceData();
    ~FenceData();

    void SetCallback(const Callback& callback) { mCallback = callback; }

    bool Init();
    void Release();

    uint64 Signal(ID3D12CommandQueue* queue, FencePtr* outFencePtr = nullptr);

    uint64 GetCompletedValue() const;

private:

    void OnValueCompleted(uint64 completedValue);

    Callback mCallback;

    D3DPtr<ID3D12Fence> mFenceObject;

    Common::RWLock mLock;
    std::atomic<uint64> mLastSignaledValue;
    uint64 mLastCompletedValue;
};


class FenceManager
{
public:
    FenceManager();
    ~FenceManager();

    bool Initialize();
    void Uninitialize();

    void RegisterFenceData(const FenceData* fenceData);
    void UnregisterFenceData(const FenceData* fenceData);
    void OnFenceRequested(FenceData* fenceData, uint64 value, const FencePtr& fence);

private:

    struct PendingFences
    {
        uint64 value;
        FenceData* fenceData;
        Common::DynArray<FenceWeakPtr> fences;
    };

    alignas(NFE_CACHE_LINE_SIZE)
    Common::RWLock mFenceDataLock;
    Common::DynArray<const FenceData*> mFenceData;

    alignas(NFE_CACHE_LINE_SIZE)
    Common::RWLock mPendingFencesLock;
    Common::DynArray<PendingFences> mPendingFences;

    alignas(NFE_CACHE_LINE_SIZE)
    Common::Thread mThread;
    std::atomic<bool> mFinish;
    HANDLE mLoopEvent;
    HANDLE mFenceWaitEvent;

    void Loop();

    void FlushFinishedFences();
};


} // namespace Renderer
} // namespace NFE
