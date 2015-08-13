/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   OGL4 Shader definitions
 */

#include "PCH.hpp"

#include "Shader.hpp"
#include "Defines.hpp"
#include "Translations.hpp"

#include "../nfCommon/File.hpp"

namespace NFE {
namespace Renderer {

Shader::Shader()
    : mType(ShaderType::Unknown)
    , mShaderProgram(0)
{
}

Shader::~Shader()
{
    glDeleteProgram(mShaderProgram);
}

bool Shader::Init(const ShaderDesc& desc)
{
    // TODO macros
    std::vector<char> shaderStr;

    mType = desc.type;

    const char* code;
    if (desc.code)
    {
        code = desc.code;
    }
    else
    {
        if (!desc.path)
        {
            LOG_ERROR("Shader code or path must be supplied");
            return false;
        }

        using namespace Common;
        File shaderFile(desc.path, AccessMode::Read);
        size_t shaderSize = static_cast<size_t>(shaderFile.GetSize());
        shaderStr.resize(shaderSize+1);

        if (shaderFile.Read(shaderStr.data(), shaderSize) != shaderSize)
        {
            LOG_ERROR("Unable to read shader code.");
            return false;
        }

        shaderStr[shaderSize] = 0;
        code = shaderStr.data();
    }

    mShaderProgram = glCreateShaderProgramv(TranslateShaderType(mType), 1, &code);
    if (!mShaderProgram)
    {
        LOG_ERROR("Unable to create Shader Program for shader '%s'", desc.path);
        return false;
    }

    int logLength = 0;
    glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    // When empty, Info Log contains only \0 char (thus the length is 1)
    if (logLength > 1)
    {
        std::vector<char> log(logLength);
        glGetProgramInfoLog(mShaderProgram, logLength, &logLength, log.data());

        LOG_INFO("Shader '%s' compilation output:\n%s", desc.path, log.data());
    }

    int programStatus = 0;
    glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &programStatus);
    if (!programStatus)
    {
        LOG_ERROR("Shader '%s' failed to compile.", desc.path);
        return false;
    }

    LOG_INFO("'%s' - compiled successfully", desc.path);

    return true;
}

GLuint Shader::GetShaderProgram()
{
    return mShaderProgram;
}

bool Shader::GetIODesc(ShaderIODesc& result)
{
    UNUSED(result);
    return false;
}

bool Shader::Disassemble(bool html, std::string& output)
{
    UNUSED(html);
    UNUSED(output);
    return false;
}


ShaderProgram::ShaderProgram()
    : mProgramPipeline(0)
{
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgramPipelines(1, &mProgramPipeline);
}

bool ShaderProgram::Init(const ShaderProgramDesc& desc)
{
    if (desc.vertexShader == nullptr || desc.pixelShader == nullptr)
    {
        LOG_ERROR("Shader Program should contain at least Vertex and Pixel shaders.");
        return false;
    }

    // generate a program pipeline
    glGenProgramPipelines(1, &mProgramPipeline);
    if (!mProgramPipeline)
    {
        LOG_ERROR("Failed to generate Program Pipeline");
        return false;
    }

    // bind existing shaders to it
    if (desc.vertexShader)
    {
        Shader* vs = dynamic_cast<Shader*>(desc.vertexShader);
        glUseProgramStages(mProgramPipeline, GL_VERTEX_SHADER_BIT, vs->mShaderProgram);
    }

    if (desc.hullShader)
    {
        Shader* hs = dynamic_cast<Shader*>(desc.hullShader);
        glUseProgramStages(mProgramPipeline, GL_TESS_CONTROL_SHADER_BIT, hs->mShaderProgram);
    }

    if (desc.domainShader)
    {
        Shader* ds = dynamic_cast<Shader*>(desc.domainShader);
        glUseProgramStages(mProgramPipeline, GL_TESS_EVALUATION_SHADER_BIT, ds->mShaderProgram);
    }

    if (desc.geometryShader)
    {
        Shader* gs = dynamic_cast<Shader*>(desc.geometryShader);
        glUseProgramStages(mProgramPipeline, GL_GEOMETRY_SHADER_BIT, gs->mShaderProgram);
    }

    if (desc.pixelShader)
    {
        Shader* ps = dynamic_cast<Shader*>(desc.pixelShader);
        glUseProgramStages(mProgramPipeline, GL_FRAGMENT_SHADER_BIT, ps->mShaderProgram);
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
