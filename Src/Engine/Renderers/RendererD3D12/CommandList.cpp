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


InternalCommandList::InternalCommandList(uint32 id)
    : mFrameNumber(UINT64_MAX)
    , mState(State::Invalid)
    , mID(id)
{ }

InternalCommandList::~InternalCommandList()
{
    NFE_ASSERT(mState == State::Free || mState == State::Invalid, "Destroying command list that is being used");
}

bool InternalCommandList::Init()
{
    HRESULT hr;
    ID3D12Device* device = gDevice->GetDevice();

    hr = D3D_CALL_CHECK(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mCommandAllocator.GetPtr())));
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Failed to create D3D12 command allocator");
        return false;
    }

    // create D3D command list
    hr = D3D_CALL_CHECK(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(),
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

void InternalCommandList::OnExecuted()
{
    NFE_ASSERT(mFrameNumber < UINT64_MAX);
    NFE_ASSERT(mState == State::Executing);

    uint64 completedValue = gDevice->mGraphicsQueueFence.fenceObject->GetCompletedValue();
    NFE_ASSERT(completedValue >= mFrameNumber);

    mReferencedResources.Clear();
    mState = State::Free;
    mFrameNumber = UINT64_MAX;
}

} // namespace Renderer
} // namespace NFE
