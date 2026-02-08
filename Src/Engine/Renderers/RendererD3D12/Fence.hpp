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

class Fence : public IFence
{
public:
    static constexpr uint64_t InvalidValue = UINT64_MAX;

    Fence(uint64 fenceValue, const FenceFlags flags, ID3D12Fence* fenceObject);
    ~Fence();

    NFE_FORCE_INLINE uint64 GetValue() const { return mFenceValue; }
    NFE_FORCE_INLINE ID3D12Fence* GetD3DFence() const { return mFenceObject; }
    NFE_FORCE_INLINE FenceFlags GetFlags() const { return mFlags; }

    virtual bool IsFinished() const override;
    virtual void Sync(Common::TaskBuilder& taskBuilder) override;

    void OnFenceFinished();

private:
    ID3D12Fence* mFenceObject; // TODO command queue - SharedPtr/WeakPtr
    uint64 mFenceValue;
    Common::TaskID mDependencyTask;
    const FenceFlags mFlags;
    std::atomic<bool> mIsFinished;
    Common::RWLock mLock;
};

} // namespace Renderer
} // namespace NFE
