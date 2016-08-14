/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"
#include "../../nfCommon/nfCommon.hpp"

#include <string>

namespace NFE {
namespace Renderer {

/**
 * Type of resource that can be bound to a shader
 */
enum class ShaderResourceType
{
    Unknown = -1,
    CBuffer = 0,
    Texture,
    Sampler,
    // TODO: UAVs, etc.
};

/**
 * Shader macro definition.
 */
struct ShaderMacro
{
    const char* name;
    const char* value;

    ShaderMacro(const char* name = nullptr, const char* value = nullptr)
        : name(name), value(value)
    { }
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
    const ShaderMacro* macros;
    size_t macrosNum;

    // TODO: binary format support

    ShaderDesc()
        : type(ShaderType::Unknown)
        , code(nullptr)
        , path(nullptr)
        , macros(nullptr)
        , macrosNum(0)
    {}
};

class IShader
{
public:
    virtual ~IShader() {}
    // TODO: binary format support (saving compiled shader bytecode)

    /**
     * Get assembly code of the shader.
     * @param html        Generate a colorful HTML document.
     * @param output[out] Output string.
     */
    virtual bool Disassemble(bool html, std::string& output) = 0;

    /**
     * Get shader resource (texture, buffer, etc.) slot ID by name.
     * @return Negative value if resource is not found.
     */
    virtual int GetResourceSlotByName(const char* name) = 0;
};

} // namespace Renderer
} // namespace NFE
