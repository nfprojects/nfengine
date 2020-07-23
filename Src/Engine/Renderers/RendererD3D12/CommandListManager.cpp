/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of CommandListManager
 */

#include "PCH.hpp"
#include "CommandListManager.hpp"
#include "RendererD3D12.hpp"

#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/Utils/ScopedLock.hpp"
#include "Engine/Common/Containers/StaticArray.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;

CommandListManager::CommandListManager()
{
}

CommandListManager::~CommandListManager()
{
    NFE_SCOPED_LOCK(mLock);

#ifndef NFE_CONFIGURATION_FINAL
    for (const InternalCommandListPtr& commandList : mCommandLists)
    {
        NFE_ASSERT(commandList->GetState() == InternalCommandList::State::Free, "All command list must be free before destroying the command list manager");
    }
#endif // NFE_CONFIGURATION_FINAL
}

InternalCommandListPtr CommandListManager::RequestCommandList()
{
    HRESULT hr;

    InternalCommandListPtr foundCommandList;
    {
        NFE_SCOPED_LOCK(mLock);

        // find a free command list
        for (const InternalCommandListPtr& commandList : mCommandLists)
        {
            if (commandList->GetState() == InternalCommandList::State::Free)
            {
                foundCommandList = commandList;
                break;
            }
        }

        if (!foundCommandList)
        {
            // command list not found - create a new one
            InternalCommandListPtr newCommandList = Common::MakeUniquePtr<InternalCommandList>(mCommandLists.Size());
            if (!newCommandList->Init())
            {
                return nullptr;
            }

            foundCommandList = newCommandList;
            mCommandLists.PushBack(std::move(newCommandList));
            return foundCommandList;
        }

        NFE_ASSERT(foundCommandList->mState == InternalCommandList::State::Free, "Invalid command list state");
        foundCommandList->mState = InternalCommandList::State::Recording;
    }

    hr = D3D_CALL_CHECK(foundCommandList->GetD3DCommandAllocator()->Reset());
    if (FAILED(hr))
    {
        return nullptr;
    }

    hr = D3D_CALL_CHECK(foundCommandList->GetD3DCommandList()->Reset(foundCommandList->GetD3DCommandAllocator(), nullptr));
    if (FAILED(hr))
    {
        return nullptr;
    }

    return foundCommandList;
}

CommandListPtr CommandListManager::OnCommandListRecorded(const InternalCommandListPtr& commandList)
{
    {
        NFE_SCOPED_LOCK(mLock);

        // transition to "recorded" state
        NFE_ASSERT(commandList->mState == InternalCommandList::State::Recording, "Invalid command list state");
        commandList->mState = InternalCommandList::State::Recorded;
    }

    return MakeUniquePtr<CommandList>(commandList);
}

void CommandListManager::ExecuteCommandList(const Common::ArrayView<ICommandList*> commandLists)
{
    const uint32 maxCommandLists = 256;
    StaticArray<ID3D12CommandList*, maxCommandLists> d3dCommandListsToExecute;

    {
        NFE_SCOPED_LOCK(mLock);

        for (ICommandList* commandList : commandLists)
        {
            NFE_ASSERT(commandList != nullptr, "Invalid command list ptr");
            CommandList* typedCommandList = static_cast<CommandList*>(commandList);

            InternalCommandListPtr internalCommandList = typedCommandList->internalCommandList.Lock();
            NFE_ASSERT(internalCommandList != nullptr, "Same commandlist cannot be executed twice");
            NFE_ASSERT(internalCommandList->GetState() == InternalCommandList::State::Recorded, "Invalid command list State. This indicates reuse from previous frame or data corruption");

            if (ID3D12GraphicsCommandList* injectedResourceBarriersCommandList = internalCommandList->GenerateResourceBarriersCommandList())
            {
                d3dCommandListsToExecute.PushBack(injectedResourceBarriersCommandList);
            }

            d3dCommandListsToExecute.PushBack(internalCommandList->GetD3DCommandList());

            internalCommandList->ApplyFinalResourceStates();
            internalCommandList->mState = InternalCommandList::State::Executing;
        }
    }

    // TODO multiple queues support
    gDevice->GetGraphicsQueue()->ExecuteCommandLists(d3dCommandListsToExecute.Size(), d3dCommandListsToExecute.Data());
}

void CommandListManager::OnFenveValueCompleted(uint64 fenceValue)
{
    NFE_ASSERT(fenceValue != FenceData::InvalidValue, "Invalid fence value");

    NFE_SCOPED_LOCK(mLock);

    for (const InternalCommandListPtr& commandList : mCommandLists)
    {
        if (commandList->GetState() == InternalCommandList::State::Executing)
        {
            if (commandList->mFenceValue <= fenceValue)
            {
                commandList->OnExecuted();
            }
        }
    }
}

} // namespace Renderer
} // namespace NFE
