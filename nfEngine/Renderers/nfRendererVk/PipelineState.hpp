/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan pipeline states.
 */

#pragma once

#include "../RendererInterface/PipelineState.hpp"
#include "Defines.hpp"
#include "ResourceBinding.hpp"

#include <tuple>


namespace NFE {
namespace Renderer {

typedef std::tuple<IPipelineState*, IShaderProgram*> FullPipelineStateParts;

class PipelineState : public IPipelineState
{
    friend class CommandBuffer;

    PipelineStateDesc mDesc;
    VkPipeline mPipeline;

public:
    ~PipelineState();
    bool Init(const PipelineStateDesc& desc);

    static VkPipeline CreateFullPipelineState(const FullPipelineStateParts& parts, const VkRenderPass rp);
};

} // namespace Renderer
} // namespace NFE
