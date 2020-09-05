#pragma once

#include "Common.hpp"
#include "D3D12MemAlloc.h"
#include "Engine/Common/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {


enum class BufferMode : uint8;


struct ResourceState
{
    // "global state" means "all subresource states are the same"
    bool isGlobalState;
    D3D12_RESOURCE_STATES globalState;

    Common::DynArray<D3D12_RESOURCE_STATES> subresourceStates;

    ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON)
        : globalState(state), isGlobalState(true) {}
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


/**
 * Generic D3D12 resource.
 */
class Resource
{
    friend class RenderTarget;
    friend class ResourceBindingInstance;
    friend class InternalCommandList;

public:
    virtual ~Resource() = default;

    NFE_FORCE_INLINE virtual ID3D12Resource* GetD3DResource() const
    {
        return mResource.Get();
    }

    NFE_FORCE_INLINE BufferMode GetMode() const
    {
        return mMode;
    }

    NFE_FORCE_INLINE const ResourceState& GetState() const
    {
        return mState;
    }

protected:
    NFE_INLINE explicit Resource(const D3D12_RESOURCE_STATES defaultState)
        : mState(defaultState)
    { }

    D3DPtr<ID3D12Resource> mResource;
    D3DPtr<D3D12MA::Allocation> mAllocation;
    ResourceState mState;
    BufferMode mMode;
};

using ResourcePtr = Common::SharedPtr<Resource>;

} // namespace Renderer
} // namespace NFE
