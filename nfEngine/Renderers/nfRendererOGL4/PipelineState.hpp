/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OGL4 Renderer's pipeline states declarations
 */

#pragma once

#include "../RendererInterface/PipelineState.hpp"

namespace NFE {
namespace Renderer {

class PipelineState : public IPipelineState
{
    friend class CommandBuffer;
    PipelineStateDesc mDesc;

public:
    bool Init(const PipelineStateDesc& desc);
};

} // namespace Renderer
} // namepsace NFE
