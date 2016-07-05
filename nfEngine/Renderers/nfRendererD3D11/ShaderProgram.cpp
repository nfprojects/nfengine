/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's shader program
 */

#include "PCH.hpp"
#include "Shader.hpp"
#include "RendererD3D11.hpp"
#include "nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

const int SHADER_TYPE_BIT_OFFSET = 24;
const int SHADER_RES_TYPE_BIT_OFFSET = 16;
const int SHADER_RES_SLOT_MASK = 0xFFFF;

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
    // the shader program slot is built as follows:
    // bits 31-24 - bitfield describing which shader stages the resource is mapped to
    // bits 23-16 - shader resource type identifier
    // bits 15-0  - shader slot
    int shaderProgramSlot = 0;
    Shader::ResBinding binding;

    // find binding in each shader stage
    for (int typeID = 0; typeID < static_cast<int>(ShaderType::Count); ++typeID)
    {
        ShaderType shaderType = static_cast<ShaderType>(typeID);
        Shader* shader = dynamic_cast<Shader*>(mDesc.GetShaderByType(shaderType));
        if (!shader)
            continue;

        const auto& bindings = shader->mResBindings;
        auto iter = bindings.find(name);
        if (iter != bindings.end())
        {
            if (binding.type != ShaderResourceType::Unknown)
            {
                if (binding.slot != iter->second.slot)
                {
                    LOG_ERROR("Shader resource '%s' has mismatched slot across shader stages");
                    return -1;
                }

                if (binding.type != iter->second.type)
                {
                    LOG_ERROR("Shader resource '%s' has mismatched type across shader stages");
                    return -1;
                }
            }
            else
                binding = iter->second;

            shaderProgramSlot |= (1 << typeID) << SHADER_TYPE_BIT_OFFSET;
        }
    }

    // binding not found in any shader stages
    if (binding.type == ShaderResourceType::Unknown)
        return -1;

    // build final slot ID
    shaderProgramSlot |= static_cast<int>(binding.type) << SHADER_RES_TYPE_BIT_OFFSET;
    shaderProgramSlot |= binding.slot;
    return shaderProgramSlot;
}

} // namespace Renderer
} // namespace NFE
