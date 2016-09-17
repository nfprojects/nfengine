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
    UNUSED(device);
    return true;
}

CommandListManager::~CommandListManager()
{
}

CommandList* CommandListManager::RequestCommandList(ID3D12CommandAllocator* commandAllocator)
{
    CommandList* foundCommandList = nullptr;
    {
        ScopedExclusiveLock lock(mLock);

        // find a free command list
        for (const CommandListPtr& commandList : mCommandLists)
        {
            if (commandList->GetState() == CommandList::State::Free)
            {
                foundCommandList = commandList.get();
                break;
            }
        }

        if (!foundCommandList)
        {
            // command list not found - create a new one
            CommandListPtr newCommandList(new CommandList);
            if (!newCommandList->Init(commandAllocator))
            {
                return nullptr;
            }

            foundCommandList = newCommandList.get();
            mCommandLists.PushBack(std::move(newCommandList));
            return foundCommandList;
        }

        NFE_ASSERT(foundCommandList->mState == CommandList::State::Free, "Invalid command list state");
        foundCommandList->mState = CommandList::State::Recording;
    }

    HRESULT hr = D3D_CALL_CHECK(foundCommandList->GetD3DCommandList()->Reset(commandAllocator, nullptr));
    if (FAILED(hr))
        return false;

    return foundCommandList;
}

bool CommandListManager::OnExecuteCommandList(CommandListID id)
{
    // TODO shared?
    ScopedExclusiveLock lock(mLock);

    if (id > mMapping.Size())
    {
        LOG_ERROR("Invalid command list ID - indicates reuse from previous frame or data corruption");
        return nullptr;
    }

    // TODO
}

bool CommandListManager::OnFinishFrame(CommandListID id)
{
    // TODO
    UNUSED(id);

    return false;
}

} // namespace Renderer
} // namespace NFE
