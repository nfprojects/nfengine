#pragma once

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/System/Thread.hpp"
#include "Engine/Common/System/ConditionVariable.hpp"
#include "Engine/Common/System/RWLock.hpp"

#include "API/Fence.hpp"


namespace NFE {
namespace Renderer {

class FenceSignaller
{
    Common::Thread mThread;
    Common::RWLock mLock;
    std::atomic<bool> mDone;
    Common::DynArray<FenceDataPtr> mRegisteredFences;

    void ThreadMain();

public:
    FenceSignaller();
    ~FenceSignaller();

    bool Init();
    void RegisterFence(const FenceDataPtr& fence);
    void UnregisterFence(const FenceDataPtr& fence);
    void Release();
};

} // namespace Renderer
} // namespace NFE
