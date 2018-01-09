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

namespace {

const std::string DEFINE_STR = "#define ";

// TODO this might be only a temporary solution. Investigate in the future.
//      The problem is, #version *must* be the first line in shader code.
//      If more flexibility is needed in this situation, some additional
//      parsing to shader code might be necessary.
const std::string SHADER_HEADER = "#version 330 core\n\
#extension GL_ARB_separate_shader_objects: enable\n\
#extension GL_ARB_shading_language_420pack : enable\n";

const std::string SHADER_HEADER_TAIL = "#line 0 1\n";

} // namespace

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
    mType = desc.type;

    // construct a shader string containing all the macros
    std::string shaderHead = SHADER_HEADER;
    if (desc.macrosNum > 0)
    {
        for (unsigned int i = 0; i < desc.macrosNum; ++i)
            shaderHead += DEFINE_STR + desc.macros[i].name + ' ' + desc.macros[i].value + '\n';
    }

    // null-termination for easier work with glShaderSource
    shaderHead += SHADER_HEADER_TAIL + '\0';

    const char* shaderCode;
    std::string shaderStr;
    if (desc.code)
    {
        shaderCode = desc.code;
    }
    else
    {
        if (!desc.path)
        {
            LOG_ERROR("Shader code or path must be supplied.");
            return false;
        }

        using namespace Common;
        File shaderFile(desc.path, AccessMode::Read);
        if (!shaderFile.IsOpened())
        {
            LOG_ERROR("Failed to open Shader file '%s'", desc.path);
            return false;
        }

        size_t shaderSize = static_cast<size_t>(shaderFile.GetSize());
        shaderStr.resize(shaderSize + 1);

        if (shaderFile.Read(&shaderStr.front(), shaderSize) != shaderSize)
        {
            LOG_ERROR("Unable to read shader code.");
            return false;
        }

        shaderStr[shaderSize] = 0;
        shaderCode = shaderStr.c_str();
    }

    // create old-style shader
    mShader = glCreateShader(TranslateShaderTypeToGLMacro(mType));
    if (!mShader)
    {
        LOG_ERROR("Unable to create Shader '%s'", desc.path);
        return false;
    }

    const char* shaderCStr[] = { shaderHead.c_str(), shaderCode };
    glShaderSource(mShader, 2, shaderCStr, nullptr);
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

    // after successful link we can detach the shader from shader program
    glDetachShader(mShaderProgram, mShader);

    LOG_SUCCESS("'%s' - Initialized successfully", desc.path);

    return true;
}

bool Shader::GetIODesc(ShaderIODesc& result)
{
    NFE_UNUSED(result);
    return false;
}

bool Shader::Disassemble(bool html, std::string& output)
{
    NFE_UNUSED(html);
    NFE_UNUSED(output);
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
        LOG_SUCCESS("Shader Program linked successfully.");

    return true;
}

} // namespace Renderer
} // namespace NFE
