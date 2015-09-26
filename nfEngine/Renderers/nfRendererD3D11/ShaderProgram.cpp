/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's shader program
 */

#include "PCH.hpp"
#include "Shader.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

ShaderProgram::ShaderProgram(const ShaderProgramDesc& desc)
    : mDesc(desc)
{
    std::map<std::string, ShaderProgramResBinding> textures;
    std::map<std::string, ShaderProgramResBinding> cbuffers;
    std::map<std::string, ShaderProgramResBinding> samplers;

    if (desc.vertexShader)
    {
        Shader* shader = dynamic_cast<Shader*>(desc.vertexShader);
        for (const auto& binding : shader->mResBindings.cbuffers)
            cbuffers[binding.first].vertexShaderSlot = binding.second;
        for (const auto& binding : shader->mResBindings.textures)
            textures[binding.first].vertexShaderSlot = binding.second;
        for (const auto& binding : shader->mResBindings.samplers)
            samplers[binding.first].vertexShaderSlot = binding.second;
    }

    if (desc.pixelShader)
    {
        Shader* shader = dynamic_cast<Shader*>(desc.pixelShader);
        for (const auto& binding : shader->mResBindings.cbuffers)
            cbuffers[binding.first].pixelShaderSlot = binding.second;
        for (const auto& binding : shader->mResBindings.textures)
            textures[binding.first].pixelShaderSlot = binding.second;
        for (const auto& binding : shader->mResBindings.samplers)
            samplers[binding.first].pixelShaderSlot = binding.second;
    }

    // TODO: other shader types

    for (auto& it : textures)
    {
        ShaderProgramResBinding& binding = it.second;
        binding.name = it.first;
        mResBinding.textures.push_back(binding);
    }

    for (auto& it : cbuffers)
    {
        ShaderProgramResBinding& binding = it.second;
        binding.name = it.first;
        mResBinding.cbuffers.push_back(binding);
    }

    for (auto& it : samplers)
    {
        ShaderProgramResBinding& binding = it.second;
        binding.name = it.first;
        mResBinding.samplers.push_back(binding);
    }
}

const ShaderProgramDesc& ShaderProgram::GetDesc() const
{
    return mDesc;
}

int ShaderProgram::GetTextureSlotByName(const char* textureName)
{
    for (size_t i = 0; i < mResBinding.textures.size(); ++i)
        if (mResBinding.textures[i].name == textureName)
            return static_cast<int>(i);
    return -1;
}

int ShaderProgram::GetCBufferSlotByName(const char* cbufferName)
{
    for (size_t i = 0; i < mResBinding.cbuffers.size(); ++i)
        if (mResBinding.cbuffers[i].name == cbufferName)
            return static_cast<int>(i);
    return -1;
}

int ShaderProgram::GetSamplerSlotByName(const char* samplerName)
{
    for (size_t i = 0; i < mResBinding.samplers.size(); ++i)
        if (mResBinding.samplers[i].name == samplerName)
            return static_cast<int>(i);
    return -1;
}

} // namespace Renderer
} // namespace NFE
