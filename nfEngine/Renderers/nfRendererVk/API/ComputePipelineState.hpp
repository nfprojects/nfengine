/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan render's compute pipeline states.
 */

#pragma once

#include "../RendererInterface/ComputePipelineState.hpp"

#include "nfCommon/Containers/SharedPtr.hpp"
#include "Defines.hpp"


namespace NFE {
namespace Renderer {

class ComputePipelineState : public IComputePipelineState
{
    DevicePtr mDevicePtr;

public:
    ComputePipelineState();
    ~ComputePipelineState();

    bool Init(DevicePtr& device, const ComputePipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
