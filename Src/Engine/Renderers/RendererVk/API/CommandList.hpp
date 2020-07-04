#pragma once

#include "../RendererCommon/CommandRecorder.hpp"


namespace NFE {
namespace Renderer {

class CommandList: public ICommandList
{
    friend class Device;

    uint32 mCommandBufferID;

public:
    CommandList(uint32 commandBufferID)
        : mCommandBufferID(commandBufferID)
    {
    }
};

} // namespace Renderer
} // namespace NFE
