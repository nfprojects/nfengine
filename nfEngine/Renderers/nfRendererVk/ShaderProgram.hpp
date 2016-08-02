/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan ShaderProgram declarations
 */

#pragma once

#include "../RendererInterface/Shader.hpp"
#include "Defines.hpp"


namespace NFE {
namespace Renderer {

class ShaderProgram : public IShaderProgram
{
    typedef std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStages;
    PipelineShaderStages mShaderStages;

public:
    ShaderProgram(const ShaderProgramDesc& desc);

    const PipelineShaderStages& GetShaderStages() const;
    int GetResourceSlotByName(const char* name) override;
};

} // namespace Renderer
} // namespace NFE
