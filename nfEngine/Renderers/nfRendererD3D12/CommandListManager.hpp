/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of CommandListManager
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"
#include "Common.hpp"
#include "CommandList.hpp"


namespace NFE {
namespace Renderer {

class CommandListManager
{
    friend class Device;

    uint64 mFrameCounter;       // total frame counter
    uint32 mFrameBufferIndex;   // current frame (command allocator index)

    std::vector<D3DPtr<ID3D12CommandAllocator>> mCommandAllocators;
    std::vector<D3DPtr<ID3D12GraphicsCommandList>> mCommandLists;

    // synchronization objects
    D3DPtr<ID3D12Fence> mFence;
    HANDLE mFenceEvent;
    std::vector<uint64> mFenceValues;

    std::mutex mMutex;

public:
    CommandListManager();
    ~CommandListManager();

    bool Init(ID3D12Device* device);

    CommandListPtr ObtainCommandList();

    bool OnFinishCommandList(CommandList* commandList);

    bool OnDestroyCommandList(CommandList* commandList);
};

} // namespace Renderer
} // namespace NFE
