/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"
#include <map>

namespace NFE {
namespace Renderer {


/**
 * Type of resource that can be bound to a shader
 */
enum class ShaderResourceType
{
    CBuffer,
    Texture,
    Sampler,
    // TODO: UAVs, etc.
};

/**
 * Shader resource binding description
 */
struct ShaderResBindingDesc
{
    int slot;
    ShaderResourceType type;
    // TODO: cbuffer structure description
};

/**
 * Shader input/outputs description.
 */
struct ShaderIODesc
{
    std::map<std::string, ShaderResBindingDesc> resBinding;
    // TODO: Vertex Shader inputs parsing
};

/**
 * Shader description used to create and compile a shader object.
 *
 * @details When @p code member is null then the shader source is loaded from @p path file.
 */
struct ShaderDesc
{
    ShaderType type;
    const char* code; //< shader code (optional)
    const char* path; //< shader path or name (optional)

    // TODO: binary format support
    // TODO: macros

    ShaderDesc()
        : type(ShaderType::Unknown)
        , code(nullptr)
        , path(nullptr)
    {}
};

class IShader
{
public:
    virtual ~IShader() {}
    // TODO: binary format support (saving compiled shader bytecode)

    /**
     * Parse shader and get inputs/outputs description.
     * @param[out] result Returned shader inputs/outputs description
     */
    virtual bool GetIODesc(ShaderIODesc& result) = 0;

    /**
     * Get assembly code of the shader.
     * @param html        Generate a colorful HTML document.
     * @param output[out] Output string.
     */
    virtual bool Disassemble(bool html, std::string& output) = 0;
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
