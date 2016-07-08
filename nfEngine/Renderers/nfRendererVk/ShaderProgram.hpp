/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan ShaderProgram declarations
 */

#include "../RendererInterface/Shader.hpp"
#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class ShaderProgram : public IShaderProgram
{
    ShaderProgramDesc mDesc;

public:
    ShaderProgram(const ShaderProgramDesc& desc);
    const ShaderProgramDesc& GetDesc() const;
    int GetResourceSlotByName(const char* name) override;
};

} // namespace Renderer
} // namespace NFE
