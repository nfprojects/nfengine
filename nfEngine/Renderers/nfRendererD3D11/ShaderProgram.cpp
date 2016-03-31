/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's shader program
 */

#include "PCH.hpp"
#include "Shader.hpp"
#include "RendererD3D11.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Renderer {

const int SHADER_TYPE_BIT_OFFSET = 24;
const int SHADER_RES_TYPE_BIT_OFFSET = 16;

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
    // the shader program slot is build as follows:
    // bits 31-24 - bitfield describing which shader stages the resource is mapped to
    // bits 23-16 - shader resource type identifier
    // bits 15-0  - shader slot
    int shaderProgramSlot = 0;
    Shader::ResBinding binding;

    const int VERTEX_SHADER_MASK = 1 << 0;
    const int HULL_SHADER_MASK = 1 << 1;
    const int DOMAIN_SHADER_MASK = 1 << 2;
    const int GEOMETRY_SHADER_MASK = 1 << 3;
    const int PIXEL_SHADER_MASK = 1 << 4;

    // find binding in each shader stage

    if (mDesc.vertexShader)
    {
        Shader* shader = dynamic_cast<Shader*>(mDesc.vertexShader);
        const auto& bindings = shader->mResBindings;
        auto iter = bindings.find(name);
        if (iter != bindings.end())
        {
            binding = iter->second;
            shaderProgramSlot |= VERTEX_SHADER_MASK << SHADER_TYPE_BIT_OFFSET;
        }
    }

    if (mDesc.hullShader)
    {
        Shader* shader = dynamic_cast<Shader*>(mDesc.hullShader);
        const auto& bindings = shader->mResBindings;
        auto iter = bindings.find(name);
        if (iter != bindings.end())
        {
            if (binding.type != ShaderResourceType::Unknown)
            {
                if (binding.slot != iter->second.slot)
                    goto slotMismatchOccurred;
                if (binding.type != iter->second.type)
                    goto typeMismatchOccurred;
            }
            else
                binding = iter->second;

            shaderProgramSlot |= HULL_SHADER_MASK << SHADER_TYPE_BIT_OFFSET;
        }
    }

    if (mDesc.domainShader)
    {
        Shader* shader = dynamic_cast<Shader*>(mDesc.domainShader);
        const auto& bindings = shader->mResBindings;
        auto iter = bindings.find(name);
        if (iter != bindings.end())
        {
            if (binding.type != ShaderResourceType::Unknown)
            {
                if (binding.slot != iter->second.slot)
                    goto slotMismatchOccurred;
                if (binding.type != iter->second.type)
                    goto typeMismatchOccurred;
            }
            else
                binding = iter->second;

            shaderProgramSlot |= DOMAIN_SHADER_MASK << SHADER_TYPE_BIT_OFFSET;
        }
    }

    if (mDesc.geometryShader)
    {
        Shader* shader = dynamic_cast<Shader*>(mDesc.geometryShader);
        const auto& bindings = shader->mResBindings;
        auto iter = bindings.find(name);
        if (iter != bindings.end())
        {
            if (binding.type != ShaderResourceType::Unknown)
            {
                if (binding.slot != iter->second.slot)
                    goto slotMismatchOccurred;
                if (binding.type != iter->second.type)
                    goto typeMismatchOccurred;
            }
            else
                binding = iter->second;

            shaderProgramSlot |= GEOMETRY_SHADER_MASK << SHADER_TYPE_BIT_OFFSET;
        }
    }

    if (mDesc.pixelShader)
    {
        Shader* shader = dynamic_cast<Shader*>(mDesc.pixelShader);
        const auto& bindings = shader->mResBindings;
        auto iter = bindings.find(name);
        if (iter != bindings.end())
        {
            if (binding.type != ShaderResourceType::Unknown)
            {
                if (binding.slot != iter->second.slot)
                    goto slotMismatchOccurred;
                if (binding.type != iter->second.type)
                    goto typeMismatchOccurred;
            }
            else
                binding = iter->second;

            shaderProgramSlot |= PIXEL_SHADER_MASK << SHADER_TYPE_BIT_OFFSET;
        }
    }

    // binding not found in any shader stages
    if (binding.type == ShaderResourceType::Unknown)
        return -1;

    // build final slot ID
    shaderProgramSlot |= static_cast<int>(binding.type) << SHADER_RES_TYPE_BIT_OFFSET;
    shaderProgramSlot |= binding.slot;
    return shaderProgramSlot;

slotMismatchOccurred:
    LOG_ERROR("Shader resource '%s' has mismatched slot across shader stages");
    return -1;

typeMismatchOccurred:
    LOG_ERROR("Shader resource '%s' has mismatched type across shader stages");
    return -1;
}

} // namespace Renderer
} // namespace NFE
