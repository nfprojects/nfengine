/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan ShaderProgram definitions
 */

#include "PCH.hpp"

#include "ShaderProgram.hpp"
#include "Shader.hpp"

namespace NFE {
namespace Renderer {

ShaderProgram::ShaderProgram(const ShaderProgramDesc& desc)
{
    VkPipelineShaderStageCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    if (desc.vertexShader)
    {
        Shader* s = dynamic_cast<Shader*>(desc.vertexShader);
        info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        info.module = s->mShader;
        info.pName = "main"; // TODO
        // TODO analyze if VkSpecializationInfo would be needed for us
        mShaderStages.push_back(info);
    }

    if (desc.hullShader)
    {
        Shader* s = dynamic_cast<Shader*>(desc.hullShader);
        info.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        info.module = s->mShader;
        info.pName = "main"; // TODO
        // TODO analyze if VkSpecializationInfo would be needed for us
        mShaderStages.push_back(info);
    }

    if (desc.domainShader)
    {
        Shader* s = dynamic_cast<Shader*>(desc.domainShader);
        info.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        info.module = s->mShader;
        info.pName = "main"; // TODO
        // TODO analyze if VkSpecializationInfo would be needed for us
        mShaderStages.push_back(info);
    }

    if (desc.geometryShader)
    {
        Shader* s = dynamic_cast<Shader*>(desc.geometryShader);
        info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        info.module = s->mShader;
        info.pName = "main"; // TODO
        // TODO analyze if VkSpecializationInfo would be needed for us
        mShaderStages.push_back(info);
    }

    if (desc.pixelShader)
    {
        Shader* s = dynamic_cast<Shader*>(desc.pixelShader);
        info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        info.module = s->mShader;
        info.pName = "main"; // TODO
        // TODO analyze if VkSpecializationInfo would be needed for us
        mShaderStages.push_back(info);
    }
}

const ShaderProgram::PipelineShaderStages& ShaderProgram::GetShaderStages() const
{
    return mShaderStages;
}

int ShaderProgram::GetResourceSlotByName(const char* name)
{
    // TODO
    UNUSED(name);
    return -1;
}

} // namespace Renderer
} // namespace NFE
