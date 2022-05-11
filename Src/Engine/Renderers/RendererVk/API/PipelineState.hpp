/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan pipeline states.
 */

#pragma once

#include "BasePipelineState.hpp"
#include "Defines.hpp"


namespace NFE {
namespace Renderer {

class PipelineState : public IPipelineState, public BasePipelineState
{
public:
    PipelineState();
    ~PipelineState();
    bool Init(const PipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
