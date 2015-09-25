/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's shader
 */

#include "PCH.hpp"
#include "Shader.hpp"
#include "RendererD3D12.hpp"
#include "../../nfCommon/Logger.hpp"
#include "../../nfCommon/File.hpp"

namespace NFE {
namespace Renderer {

Shader::Shader()
{
}

Shader::~Shader()
{
}

bool Shader::Init(const ShaderDesc& desc)
{
    UNUSED(desc);
    return false;
}

bool Shader::Disassemble(bool html, std::string& output)
{
    UNUSED(html);
    UNUSED(output);
    return false;
}

void* Shader::GetShaderObject() const
{
    return nullptr; // TODO
}

ID3DBlob* Shader::GetBytecode() const
{
    return mBytecode.get();
}

bool Shader::GetIODesc(ShaderIODesc& result)
{
    UNUSED(result);
    return false;
}


} // namespace Renderer
} // namespace NFE
