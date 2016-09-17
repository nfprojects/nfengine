/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of CommandListManager
 */

#include "PCH.hpp"
#include "CommandListManager.hpp"
#include "CommandList.hpp"
#include "RendererD3D12.hpp"

#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

CommandList::CommandList()
    : mState(State::Invalid)
{ }

bool CommandList::Init(ID3D12CommandAllocator* commandAllocator)
{
    HRESULT hr;
    ID3D12Device* device = gDevice->GetDevice();

    // create D3D command list
    hr = D3D_CALL_CHECK(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator,
                                                  nullptr, IID_PPV_ARGS(&mCommandList)));
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create D3D12 command list");
        return false;
    }

    // created D3D12 command list is in recording state by default
    mState = State::Recording;

    LOG_INFO("Command list was created");
    return true;
}

CommandListID CommandList::FinishRecording()
{
    NFE_ASSERT(mState == State::Recording, "Invalid command list state");
    mState = State::Recorded;
    return mID;
}

void CommandList::OnExecute()
{
    NFE_ASSERT(mState == State::Recorded, "Invalid command list state");
    mState = State::Free;
}

} // namespace Renderer
} // namespace NFE
