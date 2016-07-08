/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan Shader definitions
 */

#include "PCH.hpp"

#include "Shader.hpp"
#include "Defines.hpp"

namespace NFE {
namespace Renderer {

Shader::Shader()
    : mType(ShaderType::Unknown)
{}

bool Shader::Init(const ShaderDesc& desc)
{
    LOG_SUCCESS("Shader '%s' compiled successfully", desc.path);
    return true;
}

bool Shader::Disassemble(bool html, std::string& output)
{
    UNUSED(html);
    UNUSED(output);
    return false;
}

bool Shader::GetIODesc()
{
    // TODO
    return false;
}

} // namespace Renderer
} // namespace NFE
