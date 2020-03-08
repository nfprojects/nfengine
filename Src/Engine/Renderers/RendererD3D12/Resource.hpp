#pragma once

#include "Common.hpp"

namespace NFE {
namespace Renderer {

/**
 * Generic D3D12 resource.
 */
class Resource
{
    friend class RenderTarget;
    friend class ResourceBindingInstance;

public:
    virtual ~Resource() = default;

    NFE_INLINE D3D12_RESOURCE_STATES GetDefaultState() const
    {
        return mDefaultState;
    }

protected:
    NFE_INLINE explicit Resource(const D3D12_RESOURCE_STATES defaultState)
        : mDefaultState(defaultState)
    { }

    // default state (between command buffers recordings)
    D3D12_RESOURCE_STATES mDefaultState;
};

using ResourcePtr = Common::SharedPtr<Resource>;

} // namespace Renderer
} // namespace NFE
