/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of CommandListManager
 */

#include "PCH.hpp"
#include "CommandListManager.hpp"
#include "CommandQueue.hpp"
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
    for (const PerQueueTypeData& perQueueTypeData : mPerQueueTypeData)
    {
        for (const InternalCommandListPtr& commandList : perQueueTypeData.commandLists)
        {
            NFE_ASSERT(commandList->GetState() == InternalCommandList::State::Free, "All command list must be free before destroying the command list manager");
        }
    }
#endif // NFE_CONFIGURATION_FINAL
}

InternalCommandListPtr CommandListManager::RequestCommandList(CommandQueueType queueType)
{
    HRESULT hr;

    PerQueueTypeData& perQueueTypeData = mPerQueueTypeData[(uint32)queueType];

    InternalCommandListPtr foundCommandList;
    {
        NFE_SCOPED_LOCK(mLock);

        // find a free command list
        for (const InternalCommandListPtr& commandList : perQueueTypeData.commandLists)
        {
            if (commandList->GetState() == InternalCommandList::State::Free)
            {
                foundCommandList = commandList;
                break;
            }
        }

        if (!foundCommandList)
        {
            const uint32 commandListNumber = perQueueTypeData.commandLists.Size();
            NFE_ASSERT(commandListNumber < 1024, "Huge amount of command lists created, isn't something leaking?");

            // command list not found - create a new one
            InternalCommandListPtr newCommandList = Common::MakeUniquePtr<InternalCommandList>(commandListNumber);
            if (!newCommandList->Init(queueType))
            {
                return nullptr;
            }

            foundCommandList = newCommandList;
            perQueueTypeData.commandLists.PushBack(std::move(newCommandList));
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

void CommandListManager::ExecuteCommandList(const CommandQueue& queue, const Common::ArrayView<ICommandList*> commandLists)
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
            
            NFE_ASSERT(queue.GetType() == internalCommandList->GetQueueType(),
                "Incompatible command list type: queue is '%s', while command list is of type '%s'",
                CommandQueueTypeToStr(queue.GetType()), CommandQueueTypeToStr(internalCommandList->GetQueueType()));

            if (ID3D12GraphicsCommandList* injectedResourceBarriersCommandList = internalCommandList->GenerateResourceBarriersCommandList())
            {
                d3dCommandListsToExecute.PushBack(injectedResourceBarriersCommandList);
            }

            d3dCommandListsToExecute.PushBack(internalCommandList->GetD3DCommandList());

            internalCommandList->ApplyFinalResourceStates();
            internalCommandList->mState = InternalCommandList::State::Executing;
        }
    }

    queue.GetQueue()->ExecuteCommandLists(d3dCommandListsToExecute.Size(), d3dCommandListsToExecute.Data());
}

void CommandListManager::OnFenceValueCompleted(const FenceData* fenceData, uint64 fenceValue)
{
    NFE_ASSERT(fenceValue != FenceData::InvalidValue, "Invalid fence value");

    NFE_SCOPED_LOCK(mLock);

    // TODO make the fence value queue-type-specific?
    for (const PerQueueTypeData& perQueueTypeData : mPerQueueTypeData)
    {
        for (const InternalCommandListPtr& commandList : perQueueTypeData.commandLists)
        {
            if (commandList->GetState() == InternalCommandList::State::Executing)
            {
                if (commandList->mFenceData == fenceData && commandList->mFenceValue <= fenceValue)
                {
                    commandList->OnExecuted();
                }
            }
        }
    }
}

} // namespace Renderer
} // namespace NFE
