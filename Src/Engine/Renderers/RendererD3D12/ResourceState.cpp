/**
 * @file
 * @author  Witek902
 * @brief   Implementation of low-level resource state
 */

#include "PCH.hpp"
#include "ResourceState.hpp"


namespace NFE {
namespace Renderer {

void ResourceState::Set(D3D12_RESOURCE_STATES d3dState)
{
    isGlobalState = true;
    globalState = d3dState;
    subresourceStates.Clear();
}

void ResourceState::Set(uint32 subresource, D3D12_RESOURCE_STATES d3dState)
{
    if (subresource == UINT32_MAX)
    {
        // force global state
        Set(d3dState);
        return;
    }

    if (isGlobalState && globalState == d3dState)
    {
        // already had this global state, do nothing
        return;
    }

    if (subresourceStates.Size() <= subresource)
    {
        subresourceStates.Resize(subresource + 1);
    }

    isGlobalState = false;
    subresourceStates[subresource] = d3dState;
}

D3D12_RESOURCE_STATES ResourceState::Get(uint32 subresource) const
{
    if (isGlobalState)
    {
        return globalState;
    }

    if (subresource >= subresourceStates.Size())
    {
        return globalState;
    }

    return subresourceStates[subresource];
}

bool ResourceState::Matches(uint32 subresource, D3D12_RESOURCE_STATES d3dState) const
{
    if (subresource == UINT32_MAX)
    {
        return isGlobalState && globalState == d3dState;
        // TODO check subresources array if all elements are the same?
    }
    else
    {
        if (isGlobalState)
        {
            return globalState == d3dState;
        }
        else
        {
            if (subresource >= subresourceStates.Size())
            {
                return globalState == d3dState;
            }
            else
            {
                return subresourceStates[subresource] == d3dState;
            }
        }
    }
}

void ResourceState::Shrink()
{
    if (!isGlobalState)
    {
        // check if can be converted to "global" state
        bool makeGlobalState = true;
        for (uint32 i = 1; i < subresourceStates.Size(); ++i)
        {
            if (subresourceStates[i] != subresourceStates[0])
            {
                makeGlobalState = false;
                break;
            }
        }

        if (makeGlobalState)
        {
            isGlobalState = true;
            globalState = subresourceStates[0];
            subresourceStates.Clear();
            return;
        }

        // TODO shrink elements at the end
    }
}

} // namespace Renderer
} // namespace NFE
