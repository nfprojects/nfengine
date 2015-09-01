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
    , mShader(GL_NONE)
    , mShaderProgram(GL_NONE)
{
}

Shader::~Shader()
{
    glDeleteShader(mShader);
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

    // create old-style shader
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

        return false;
    }
    else
        LOG_INFO("'%s' - compiled successfully", desc.path);

    // convert old-style shader to shader program with SSO enabled
    // the behavior below matches glCreateShaderProgramv's implementation
    mShaderProgram = glCreateProgram();
    glProgramParameteri(mShaderProgram, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(mShaderProgram, mShader);
    glLinkProgram(mShaderProgram);

    int programStatus = 0;
    glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &programStatus);
    if (!programStatus)
    {
        LOG_ERROR("Separable Shader Program failed to link.");

        int logLength = 0;
        glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &logLength);
        // When empty, Info Log contains only \0 char (thus the length is 1)
        if (logLength > 1)
        {
            std::vector<char> log(logLength);
            glGetProgramInfoLog(mShaderProgram, logLength, &logLength, log.data());
            LOG_INFO("Separable Shader Program linking output:\n%s", log.data());
        }
    }
    else
        LOG_INFO("Separable Shader Program linked successfully.");

    // after successful link we can detach the shader from shader program
    glDetachShader(mShaderProgram, mShader);

    return true;
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
