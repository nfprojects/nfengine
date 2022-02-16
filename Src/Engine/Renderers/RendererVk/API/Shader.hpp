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
    friend class CommandRecorder;
    friend class PipelineState;
    friend class ComputePipelineState;

    ShaderType mType;
    Common::String mShaderPath;
    VkPipelineShaderStageCreateInfo mStageInfo;
    SpvReflectShaderModule mSpvReflectModule;
    Common::DynArray<SpvReflectDescriptorSet*> mDescriptorSets;

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);

    bool Disassemble(bool html, Common::String& output) override;
    int GetResourceSlotByName(const char* name) override;
};

} // namespace Renderer
} // namespace NFE
