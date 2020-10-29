/**
 * @file
 * @author  Witek902
 * @brief   D3D12 implementation of renderer's generic resource
 */

#include "PCH.hpp"
#include "Resource.hpp"
#include "../RendererCommon/Types.hpp"


namespace NFE {
namespace Renderer {

Resource::Resource(const D3D12_RESOURCE_STATES defaultState)
    : mState(defaultState)
    , mMode(ResourceAccessMode::Invalid)
{ }

} // namespace Renderer
} // namespace NFE
