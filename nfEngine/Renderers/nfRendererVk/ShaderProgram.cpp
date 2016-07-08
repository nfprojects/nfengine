/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan ShaderProgram definitions
 */

#include "PCH.hpp"

#include "ShaderProgram.hpp"

namespace NFE {
namespace Renderer {

ShaderProgram::ShaderProgram(const ShaderProgramDesc& desc)
    : mDesc(desc)
{
}

const ShaderProgramDesc& ShaderProgram::GetDesc() const
{
    return mDesc;
}

int ShaderProgram::GetResourceSlotByName(const char* name)
{
    // TODO
    UNUSED(name);
    return -1;
}

} // namespace Renderer
} // namespace NFE
