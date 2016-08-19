/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan ShaderProgram definitions
 */

#include "PCH.hpp"

#include "ShaderProgram.hpp"
#include "Shader.hpp"
#include "Device.hpp"

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/disassemble.h>


namespace NFE {
namespace Renderer {

bool ShaderProgram::CreateShaderModule(const EShLanguage stage, VkShaderModule& outShader)
{
    // now we have spirv representation of shader, provide it to Vulkan
    glslang::TIntermediate* intermediate = mProgramGlslang->getIntermediate(stage);
    if (!intermediate)
    {
        LOG_ERROR("Unable to extract shader intermediate for language %d", stage);
        return false;
    }

    std::string errorMessages;
    spv::SpvBuildLogger spvLogger;
    std::vector<uint32> shaderSpv;
    glslang::GlslangToSpv(*intermediate, shaderSpv, &spvLogger);

    VkShaderModuleCreateInfo shaderInfo;
    VK_ZERO_MEMORY(shaderInfo);
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = shaderSpv.size() * sizeof(uint32);
    shaderInfo.pCode = shaderSpv.data();
    VkResult result = vkCreateShaderModule(gDevice->GetDevice(), &shaderInfo, nullptr, &outShader);
    CHECK_VKRESULT(result, "Failed to create Shader module");

    return true;
}

ShaderProgram::ShaderProgram()
    : mVertexShader(VK_NULL_HANDLE)
    , mTessControlShader(VK_NULL_HANDLE)
    , mTessEvalShader(VK_NULL_HANDLE)
    , mGeometryShader(VK_NULL_HANDLE)
    , mFragmentShader(VK_NULL_HANDLE)

{
}

ShaderProgram::~ShaderProgram()
{
    if (mFragmentShader != VK_NULL_HANDLE)
        vkDestroyShaderModule(gDevice->GetDevice(), mFragmentShader, nullptr);
    if (mGeometryShader != VK_NULL_HANDLE)
        vkDestroyShaderModule(gDevice->GetDevice(), mGeometryShader, nullptr);
    if (mTessEvalShader != VK_NULL_HANDLE)
        vkDestroyShaderModule(gDevice->GetDevice(), mTessEvalShader, nullptr);
    if (mTessControlShader != VK_NULL_HANDLE)
        vkDestroyShaderModule(gDevice->GetDevice(), mTessControlShader, nullptr);
    if (mVertexShader != VK_NULL_HANDLE)
        vkDestroyShaderModule(gDevice->GetDevice(), mVertexShader, nullptr);
}

bool ShaderProgram::Init(const ShaderProgramDesc& desc)
{
    VkPipelineShaderStageCreateInfo info;
    VK_ZERO_MEMORY(info);
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    mProgramGlslang.reset(new (std::nothrow) glslang::TProgram());

    // gather all shaders to our program
    if (desc.vertexShader)
    {
        Shader* s = dynamic_cast<Shader*>(desc.vertexShader);
        mProgramGlslang->addShader(s->mShaderGlslang.get());
    }

    if (desc.hullShader)
    {
        Shader* s = dynamic_cast<Shader*>(desc.hullShader);
        mProgramGlslang->addShader(s->mShaderGlslang.get());
    }

    if (desc.domainShader)
    {
        Shader* s = dynamic_cast<Shader*>(desc.domainShader);
        mProgramGlslang->addShader(s->mShaderGlslang.get());
    }

    if (desc.geometryShader)
    {
        Shader* s = dynamic_cast<Shader*>(desc.geometryShader);
        mProgramGlslang->addShader(s->mShaderGlslang.get());
    }

    if (desc.pixelShader)
    {
        Shader* s = dynamic_cast<Shader*>(desc.pixelShader);
        mProgramGlslang->addShader(s->mShaderGlslang.get());
    }


    // create a glslang program, which will be used to extract code and resource slots
    if (!mProgramGlslang->link(gShaderMessages))
    {
        LOG_ERROR("Failed to link shaders:\n%s", mProgramGlslang->getInfoLog());
        return false;
    }

    /*if (!mProgramGlslang->buildReflection())
    {
        LOG_ERROR("Failed to build program's reflection:\n%s", mProgramGlslang->getInfoLog());
        return false;
    }*/


    // create Vulkan shader modules
    if (desc.vertexShader)
    {
        if (!CreateShaderModule(EShLangVertex, mVertexShader))
        {
            LOG_ERROR("Failed to create Vulkan shader module for vertex shader");
            return false;
        }

        info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        info.pName = "main"; // TODO
        info.module = mVertexShader;
        // TODO analyze if VkSpecializationInfo would be useful for us
        mShaderStages.push_back(info);
    }

    if (desc.hullShader)
    {
        if (!CreateShaderModule(EShLangTessControl, mTessControlShader))
        {
            LOG_ERROR("Failed to create Vulkan shader module for tessellation control shader");
            return false;
        }

        info.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        info.pName = "main"; // TODO
        info.module = mTessControlShader;
        // TODO analyze if VkSpecializationInfo would be useful for us
        mShaderStages.push_back(info);
    }

    if (desc.domainShader)
    {
        if (!CreateShaderModule(EShLangTessEvaluation, mTessEvalShader))
        {
            LOG_ERROR("Failed to create Vulkan shader module for tessellation evaluation shader");
            return false;
        }

        info.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        info.pName = "main"; // TODO
        info.module = mTessEvalShader;
        // TODO analyze if VkSpecializationInfo would be useful for us
        mShaderStages.push_back(info);
    }

    if (desc.geometryShader)
    {
        if (!CreateShaderModule(EShLangGeometry, mGeometryShader))
        {
            LOG_ERROR("Failed to create Vulkan shader module for geometry shader");
            return false;
        }

        info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        info.pName = "main"; // TODO
        info.module = mGeometryShader;
        // TODO analyze if VkSpecializationInfo would be useful for us
        mShaderStages.push_back(info);
    }

    if (desc.pixelShader)
    {
        if (!CreateShaderModule(EShLangFragment, mFragmentShader))
        {
            LOG_ERROR("Failed to create Vulkan shader module for fragment shader");
            return false;
        }

        info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        info.pName = "main"; // TODO
        info.module = mFragmentShader;
        // TODO analyze if VkSpecializationInfo would be useful for us
        mShaderStages.push_back(info);
    }

    return true;
}

const ShaderProgram::PipelineShaderStages& ShaderProgram::GetShaderStages() const
{
    return mShaderStages;
}

int ShaderProgram::GetResourceSlotByName(const char* name)
{
    UNUSED(name);
    //return mProgramGlslang->getUniformIndex(name);
    return 0; // TODO VERY TEMP
}

} // namespace Renderer
} // namespace NFE
