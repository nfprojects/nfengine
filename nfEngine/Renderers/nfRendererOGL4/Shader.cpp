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
    , mShader(0)
{
}

Shader::~Shader()
{
    glDeleteShader(mShader);
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

    mShader = glCreateShader(TranslateShaderTypeToGLMacro(mType));
    if (!mShader)
    {
        LOG_ERROR("Unable to create Shader '%s'", desc.path);
        return false;
    }

    GLint codeLen = static_cast<GLint>(strlen(code));
    glShaderSource(mShader, 1, &code, &codeLen);
    glCompileShader(mShader);

    int shaderStatus = 0;
    glGetShaderiv(mShader, GL_COMPILE_STATUS, &shaderStatus);
    if (!shaderStatus)
    {
        LOG_ERROR("Failed to compile Shader '%s'", desc.path);
        int logLength = 0;
        glGetShaderiv(mShader, GL_INFO_LOG_LENGTH, &logLength);
        // When empty, Info Log contains only \0 char (thus the length is 1)
        if (logLength > 1)
        {
            std::vector<char> log(logLength);
            glGetShaderInfoLog(mShader, logLength, &logLength, log.data());

            LOG_INFO("Shader '%s' compilation output:\n%s", desc.path, log.data());
        }
    }
    else
        LOG_INFO("'%s' - compiled successfully", desc.path);

    return true;
}

GLuint Shader::GetShader()
{
    return mShader;
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
    : mProgram(0)
{
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(mProgram);
}

bool ShaderProgram::Init(const ShaderProgramDesc& desc)
{
    // generate a program pipeline
    mProgram = glCreateProgram();
    if (!mProgram)
    {
        LOG_ERROR("Failed to generate Shader Program");
        return false;
    }

    // bind existing shaders to it
    if (desc.vertexShader)
    {
        Shader* vs = dynamic_cast<Shader*>(desc.vertexShader);
        glAttachShader(mProgram, vs->mShader);
    }

    if (desc.hullShader)
    {
        Shader* hs = dynamic_cast<Shader*>(desc.hullShader);
        glAttachShader(mProgram, hs->mShader);
    }

    if (desc.domainShader)
    {
        Shader* ds = dynamic_cast<Shader*>(desc.domainShader);
        glAttachShader(mProgram, ds->mShader);
    }

    if (desc.geometryShader)
    {
        Shader* gs = dynamic_cast<Shader*>(desc.geometryShader);
        glAttachShader(mProgram, gs->mShader);
    }

    if (desc.pixelShader)
    {
        Shader* ps = dynamic_cast<Shader*>(desc.pixelShader);
        glAttachShader(mProgram, ps->mShader);
    }

    glLinkProgram(mProgram);

    int programStatus = 0;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &programStatus);
    if (!programStatus)
    {
        LOG_ERROR("Shader Program failed to link.");

        int logLength = 0;
        glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &logLength);
        // When empty, Info Log contains only \0 char (thus the length is 1)
        if (logLength > 1)
        {
            std::vector<char> log(logLength);
            glGetProgramInfoLog(mProgram, logLength, &logLength, log.data());
            LOG_INFO("Shader Program linking output:\n%s", log.data());
        }

        return false;
    }
    else
        LOG_INFO("Shader Program linked successfully.");

    return true;
}

} // namespace Renderer
} // namespace NFE
