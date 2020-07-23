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

/**
 * Synchronization object
 * Allows for waiting for some GPU commands to finish
 */
class NFE_RENDERER_COMMON_API IFence
{
public:
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
