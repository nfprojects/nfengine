/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan Shader declarations
 */

#include "../RendererInterface/Shader.hpp"
#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class Shader : public IShader
{
    friend class CommandBuffer;

    ShaderType mType;

    bool GetIODesc();

public:
    Shader();
    bool Init(const ShaderDesc& desc);

    bool Disassemble(bool html, std::string& output);
};

} // namespace Renderer
} // namespace NFE
