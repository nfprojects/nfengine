#pragma once

#include "Defines.hpp"
#include "CommandBatchState.hpp"
#include "LocalAllocatorObject.hpp"

#include "API/RenderTarget.hpp"
#include "API/ResourceBinding.hpp"

#include <Engine/Common/nfCommon.hpp>


namespace NFE {
namespace Renderer {


// Command interface

class ICommand: public CommandAllocatorObject
{
    friend class CommandRecorder;
    friend class CommandBatch;

    uint32 mPriority;

public:
    ICommand(uint32 priority)
        : mPriority(priority)
    {
    }

    virtual ~ICommand()
    {
    }

    virtual void Execute(VkCommandBuffer commandBuffer, CommandBatchState& state) = 0;
    virtual const char* GetName() const { return nullptr; }
    NFE_FORCE_INLINE uint32 GetPriority() const { return mPriority; }
};


} // namespace Renderer
} // namespace NFE
