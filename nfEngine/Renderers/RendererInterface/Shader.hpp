/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

namespace NFE {
namespace Renderer {

/**
 * Shader description.
 */
struct ShaderDesc
{
    ShaderType type;
    const char* code;
    const char* name;

    // TODO: binary format support
    // TODO: macros
};

class IShader
{
public:
    virtual ~IShader() {}
    // TODO: binary format support (saving compiled shader bytecode)
};

/**
 * Shader program description.
 */
struct ShaderProgramDesc
{
    IShader* vertexShader;
    IShader* hullShader;
    IShader* domainShader;
    IShader* geometryShader;
    IShader* pixelShader;

    ShaderProgramDesc()
        : vertexShader(nullptr)
        , hullShader(nullptr)
        , domainShader(nullptr)
        , geometryShader(nullptr)
        , pixelShader(nullptr)
    {
    }
};

/**
 * Shader program - collection of linked shaders.
 */
class IShaderProgram
{
public:
    virtual ~IShaderProgram() {}
};

} // namespace Renderer
} // namespace NFE
