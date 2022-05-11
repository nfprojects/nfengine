/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Vulkan Shader declarations
 */

#pragma once

#include "../RendererCommon/Shader.hpp"
#include "Defines.hpp"

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/Containers/String.hpp"


namespace NFE {
namespace Renderer {

class Shader : public IShader
{
public:
    using DescriptorBindings = Common::DynArray<SpvReflectDescriptorBinding*>;

private:
    ShaderType mType;
    Common::String mShaderPath;
    VkPipelineShaderStageCreateInfo mStageInfo;
    SpvReflectShaderModule mSpvReflectModule;
    DescriptorBindings mDescriptorBindings;

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);

    bool Disassemble(bool html, Common::String& output) override;
    int GetResourceSlotByName(const char* name) override;

    bool ChangeDescriptorSetBinding(const SpvReflectDescriptorBinding* binding, uint32 newSe);

    NFE_INLINE const Common::String& GetShaderPath() const
    {
        return mShaderPath;
    }

    NFE_INLINE const SpvReflectShaderModule& GetReflectShaderModule() const
    {
        return mSpvReflectModule;
    }

    NFE_INLINE const VkPipelineShaderStageCreateInfo& GetShaderStageInfo() const
    {
        return mStageInfo;
    }

    NFE_INLINE bool ForEachDescriptorBinding(ForEachFunction<SpvReflectDescriptorBinding*> f)
    {
        bool success = true;

        for (auto& binding: mDescriptorBindings)
        {
            if (!f(binding))
            {
                success = false;
                break;
            }
        }

        return success;
    }
};

} // namespace Renderer
} // namespace NFE
