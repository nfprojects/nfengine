#pragma once

#include "../RendererCommon/Fence.hpp"
#include "Engine/Common/Containers/SharedPtr.hpp"
#include "Engine/Common/System/Thread.hpp"
#include "Engine/Common/System/RWLock.hpp"
#include "Engine/Common/Utils/ThreadPool.hpp"
#include "Engine/Common/Utils/ThreadPoolTask.hpp"

#include <functional>


namespace NFE {
namespace Renderer {

class FenceData
{
public:
    union Synchronizable
    {
        VkFence fence;
        VkSemaphore semaphore;
    };

    using SignalledCallback = std::function<void()>;

    const FenceFlags flags;
    const SignalledCallback onSignalled;
    Synchronizable synchronizable;

    FenceData(const FenceFlags _flags, SignalledCallback&& _callback)
        : flags(_flags)
        , onSignalled(std::move(_callback))
        , synchronizable()
    {
    }

    ~FenceData();
    bool Init();
};

using FenceDataPtr = Common::SharedPtr<FenceData>;


class Fence: public IFence
{
private:
    Common::TaskID mSyncTask;
    std::atomic<bool> mFinished;
    Common::RWLock mLock;
    FenceDataPtr mData;

public:
    Fence(const FenceFlags flags);
    virtual ~Fence();

    bool Init();
    void OnSignalled();
    bool IsFinished() const override;
    void Sync(Common::TaskBuilder& taskBuilder) override;

    NFE_FORCE_INLINE const FenceFlags& GetFlags() const
    {
        NFE_ASSERT(mData != nullptr, "Fence not initialized, flags unavailable");
        return mData->flags;
    }

    NFE_FORCE_INLINE const FenceData::Synchronizable& GetSynchronizable() const
    {
        NFE_ASSERT(mData != nullptr, "Fence not initialized, Synchronizable unavailable");
        return mData->synchronizable;
    }
};

} // namespace Renderer
} // namespace NFE
