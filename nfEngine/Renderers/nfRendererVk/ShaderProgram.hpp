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
    std::unique_ptr<glslang::TProgram> mProgramGlslang;

    VkShaderModule mVertexShader;
    VkShaderModule mTessControlShader;
    VkShaderModule mTessEvalShader;
    VkShaderModule mGeometryShader;
    VkShaderModule mFragmentShader;

    bool CreateShaderModule(const EShLanguage stage, VkShaderModule& outShader);

public:
    ShaderProgram();
    ~ShaderProgram();

    bool Init(const ShaderProgramDesc& desc);
    const PipelineShaderStages& GetShaderStages() const;
    int GetResourceSlotByName(const char* name) override;
};

} // namespace Renderer
} // namespace NFE
