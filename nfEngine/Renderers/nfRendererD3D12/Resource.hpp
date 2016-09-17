/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 */

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

protected:

    // default state (between command buffers recordings)
    D3D12_RESOURCE_STATES mDefaultState;

public:
    virtual ~Resource() = default;

};

} // namespace Renderer
} // namespace NFE
