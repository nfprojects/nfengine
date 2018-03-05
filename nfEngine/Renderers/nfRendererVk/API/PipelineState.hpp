/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan pipeline states.
 */

#pragma once

#include "../RendererInterface/PipelineState.hpp"
#include "Defines.hpp"
#include "ResourceBinding.hpp"


namespace NFE {
namespace Renderer {

class PipelineState : public IPipelineState
{
    friend class CommandRecorder;

    DevicePtr mDevicePtr;

    PipelineStateDesc mDesc;
    VkPipeline mPipeline;

public:
    PipelineState();
    ~PipelineState();
    bool Init(DevicePtr& device, const PipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
