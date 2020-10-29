#pragma once

#include "Common.hpp"
#include "ResourceState.hpp"
#include "D3D12MemAlloc.h"


namespace NFE {
namespace Renderer {

enum class ResourceAccessMode : uint8;

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

    NFE_FORCE_INLINE ResourceAccessMode GetMode() const
    {
        return mMode;
    }

    NFE_FORCE_INLINE const ResourceState& GetState() const
    {
        return mState;
    }

protected:
    explicit Resource(const D3D12_RESOURCE_STATES defaultState);

    D3DPtr<ID3D12Resource> mResource;
    D3DPtr<D3D12MA::Allocation> mAllocation;
    ResourceState mState;
    ResourceAccessMode mMode;
};

using ResourcePtr = Common::SharedPtr<Resource>;

} // namespace Renderer
} // namespace NFE
