/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of CommandListManager
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"
#include "Common.hpp"
#include "CommandList.hpp"
#include "nfCommon/nfCommon.hpp"


namespace NFE {
namespace Renderer {

class CommandListManager;
class RingBuffer;

class CommandList final : public ICommandList
{
    NFE_MAKE_NONCOPYABLE(CommandList);

    CommandListManager* mManager;
    ID3D12GraphicsCommandList* mD3DCommandList;

public:
    RingBuffer* ringBuffer;
    uint64 fenceValue;

    uint32 commandListID;
    uint32 commandAllocatorID;

    CommandList(CommandListManager* manager, ID3D12GraphicsCommandList* d3dCommandList);
    ~CommandList();

    NFE_INLINE ID3D12GraphicsCommandList* GetD3DCommandList() const
    {
        return mD3DCommandList;
    }

};

using CommandListPtr = std::unique_ptr<CommandList>;

} // namespace Renderer
} // namespace NFE
