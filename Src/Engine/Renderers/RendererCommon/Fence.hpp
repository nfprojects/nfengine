/**
 * @file
 * @author  Witek902
 * @brief   Declarations of low-level render's IFence interface
 */

#pragma once

#include "RendererCommon.hpp"

#include "../../Common/Containers/WeakPtr.hpp"

namespace NFE {
namespace Renderer {

enum FenceFlags
{
    FenceFlag_CpuWaitable = (1<<0),
    FenceFlag_GpuWaitable = (1<<1),
};

/**
 * Synchronization object
 * Allows for waiting for some GPU commands to finish
 */
class NFE_RENDERER_COMMON_API IFence
{
    NFE_MAKE_NONCOPYABLE(IFence)
    NFE_MAKE_NONMOVEABLE(IFence)

public:
    IFence() = default;
    virtual ~IFence();

    /**
     * Check if the event already has finished.
     */
    virtual bool IsFinished() const = 0;

    /**
     * Synchronize a task builder so some CPU task can depend on the GPU event completion.
     */
    virtual void Sync(Common::TaskBuilder& taskBuilder) = 0;

    /**
     * Helper function that waits until the fence is completed.
     */
    void Wait();
};

using FencePtr = Common::SharedPtr<IFence>;
using FenceWeakPtr = Common::WeakPtr<IFence>;


} // namespace Renderer
} // namespace NFE
