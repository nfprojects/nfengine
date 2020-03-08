/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of CommandListManager
 */

#include "PCH.hpp"
#include "CommandListManager.hpp"
#include "CommandList.hpp"
#include "RendererD3D12.hpp"

#include "Engine/Common/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {


CommandList::CommandList(CommandListID id)
    : mFrameNumber(0)
    , mState(State::Invalid)
    , mID(id)
{ }

CommandList::~CommandList()
{
    NFE_ASSERT(mState == State::Free || mState == State::Invalid, "Destroying command list that is being used");
}

bool CommandList::Init(ID3D12CommandAllocator* commandAllocator)
{
    HRESULT hr;
    ID3D12Device* device = gDevice->GetDevice();

    // create D3D command list
    hr = D3D_CALL_CHECK(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator,
                                                  nullptr, IID_PPV_ARGS(mCommandList.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create D3D12 command list");
        return false;
    }

    // created D3D12 command list is in recording state by default
    mState = State::Recording;

    NFE_LOG_INFO("Command list was created, ID = %u", mID);
    return true;
}


} // namespace Renderer
} // namespace NFE
