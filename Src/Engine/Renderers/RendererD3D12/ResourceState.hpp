#pragma once

#include "Common.hpp"
#include "Engine/Common/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

struct ResourceState
{
    // "global state" means "all subresource states are the same"
    bool isGlobalState;
    D3D12_RESOURCE_STATES globalState;

    Common::DynArray<D3D12_RESOURCE_STATES> subresourceStates;

    ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON) : globalState(state), isGlobalState(true) {}
    ResourceState(const ResourceState& other) = default;
    ResourceState(ResourceState&& other) = default;
    ResourceState& operator = (const ResourceState& other) = default;
    ResourceState& operator = (ResourceState && other) = default;

    void Set(D3D12_RESOURCE_STATES d3dState);
    void Set(uint32 subresource, D3D12_RESOURCE_STATES d3dState);
    D3D12_RESOURCE_STATES Get(uint32 subresource) const;
    bool Matches(uint32 subresource, D3D12_RESOURCE_STATES d3dState) const;
    void Shrink();
};

} // namespace Renderer
} // namespace NFE
