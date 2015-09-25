/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's shader program
 */

#include "PCH.hpp"
#include "Shader.hpp"
#include "RendererD3D12.hpp"

namespace NFE {
namespace Renderer {

ShaderProgram::ShaderProgram(const ShaderProgramDesc& desc)
    : mDesc(desc)
{ }

const ShaderProgramDesc& ShaderProgram::GetDesc() const
{
    return mDesc;
}

} // namespace Renderer
} // namespace NFE
