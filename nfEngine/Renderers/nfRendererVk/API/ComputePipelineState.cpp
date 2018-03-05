/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's compute pipeline state
 */

#include "PCH.hpp"
#include "ComputePipelineState.hpp"

#include "Device.hpp"


namespace NFE {
namespace Renderer {

ComputePipelineState::ComputePipelineState()
{
}

ComputePipelineState::~ComputePipelineState()
{
}

bool ComputePipelineState::Init(DevicePtr& device, const ComputePipelineStateDesc& desc)
{
    mDevicePtr = device;

    // TODO
    NFE_UNUSED(desc);
    return false;
}

} // namespace Renderer
} // namespace NFE
