#pragma once

#include "../RendererCommon/Fence.hpp"
#include "Engine/Common/System/Thread.hpp"
#include "Engine/Common/System/RWLock.hpp"
#include "Engine/Common/Utils/ThreadPool.hpp"
#include "Engine/Common/Utils/ThreadPoolTask.hpp"


namespace NFE {
namespace Renderer {

class Fence: public IFence
{
public:
    union Synchronizable
    {
        VkFence mFence;
        VkSemaphore mSemaphore;
    };

private:
    const FenceFlags mFlags;
    Common::TaskID mSyncTask;
    std::atomic<bool> mFinished;
    Common::RWLock mLock;
    Synchronizable mSynchronizable;

public:
    Fence(const FenceFlags flags);
    virtual ~Fence();

    bool Init();

    bool IsFinished() const override;
    void Sync(Common::TaskBuilder& taskBuilder) override;

    NFE_FORCE_INLINE const FenceFlags& GetFlags() const
    {
        return mFlags;
    }

    NFE_FORCE_INLINE const Synchronizable& GetSynchronizable() const
    {
        return mSynchronizable;
    }
};

} // namespace Renderer
} // namespace NFE
