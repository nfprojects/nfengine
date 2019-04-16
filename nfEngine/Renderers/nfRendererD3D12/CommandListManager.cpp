/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of CommandListManager
 */

#include "PCH.hpp"
#include "CommandListManager.hpp"
#include "RendererD3D12.hpp"

#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/Utils/ScopedLock.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;

CommandListManager::CommandListManager()
{
}

bool CommandListManager::Init(ID3D12Device* device)
{
    NFE_UNUSED(device);
    return true;
}

CommandListManager::~CommandListManager()
{
    NFE_SCOPED_LOCK(mLock);

    for (const CommandListPtr& commandList : mCommandLists)
    {
        NFE_ASSERT(commandList->GetState() == CommandList::State::Free,
                   "All command list must be free before destroying the command list manager");
    }
}

CommandList* CommandListManager::RequestCommandList(ID3D12CommandAllocator* commandAllocator)
{
    CommandList* foundCommandList = nullptr;
    {
        NFE_SCOPED_LOCK(mLock);

        // find a free command list
        for (const CommandListPtr& commandList : mCommandLists)
        {
            if (commandList->GetState() == CommandList::State::Free)
            {
                foundCommandList = commandList.Get();
                break;
            }
        }

        if (!foundCommandList)
        {
            // command list not found - create a new one
            CommandListPtr newCommandList = Common::MakeUniquePtr<CommandList>(mCommandLists.Size());
            if (!newCommandList->Init(commandAllocator))
            {
                return nullptr;
            }

            foundCommandList = newCommandList.Get();
            mCommandLists.PushBack(std::move(newCommandList));
            return foundCommandList;
        }

        NFE_ASSERT(foundCommandList->mState == CommandList::State::Free, "Invalid command list state");
        foundCommandList->mState = CommandList::State::Recording;
    }

    HRESULT hr = D3D_CALL_CHECK(foundCommandList->GetD3DCommandList()->Reset(commandAllocator, nullptr));
    if (FAILED(hr))
        return nullptr;

    return foundCommandList;
}

CommandListID CommandListManager::OnCommandListRecorded(CommandList* commandList)
{
    NFE_SCOPED_LOCK(mLock);

    // transition to "recorded" state
    NFE_ASSERT(commandList->mState == CommandList::State::Recording, "Invalid command list state");
    commandList->mState = CommandList::State::Recorded;

    return commandList->mID;
}

bool CommandListManager::OnExecuteCommandList(CommandListID id, uint64 frameNumber)
{
    NFE_ASSERT(id != INVALID_COMMAND_LIST_ID, "Invalid command list ID");

    CommandList* commandList = nullptr;
    {
        NFE_SCOPED_LOCK(mLock);

        if (id >= mCommandLists.Size())
        {
            NFE_LOG_ERROR("Invalid command list ID. This indicates reuse from previous frame or data corruption");
            return false;
        }

        commandList = mCommandLists[id].Get();
        if (commandList->GetState() != CommandList::State::Recorded)
        {
            NFE_LOG_ERROR("Invalid command list ID. This indicates reuse from previous frame or data corruption");
            return false;
        }

        // TODO
        // 1. batching
        // 2. multiple queues support
        // 3. don't execute inside a lock
        ID3D12CommandList* commandLists[] = { commandList->GetD3DCommandList() };
        gDevice->GetCommandQueue()->ExecuteCommandLists(1, commandLists);

        // transition to "free" state
        commandList->mState = CommandList::State::Free;
        commandList->mFrameNumber = frameNumber;
    }


    return true;
}

bool CommandListManager::OnFinishFrame()
{
    bool success = true;
    NFE_SCOPED_LOCK(mLock);

    // mark all completed command lists as free
    for (const CommandListPtr& commandList : mCommandLists)
    {
        if (commandList->GetState() == CommandList::State::Recorded)
        {
            NFE_LOG_ERROR("Command list (ID = %u) is in 'recorded' state. "
                          "This is invalid - all command lists must be submitted before calling Device::FinishFrame()");
            success = false;
        }
        else if (commandList->GetState() == CommandList::State::Recording)
        {
            NFE_LOG_ERROR("Command list (ID = %u) is in 'recording' state. "
                          "This is invalid - all command lists must be submitted before calling Device::FinishFrame()");
            success = false;
        }
    }

    return success;
}


} // namespace Renderer
} // namespace NFE
