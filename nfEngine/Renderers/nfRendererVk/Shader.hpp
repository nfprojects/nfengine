/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan Shader declarations
 */

#pragma once

#include "../RendererInterface/Shader.hpp"
#include "Defines.hpp"

namespace NFE {
namespace Renderer {

class Shader : public IShader
{
    friend class CommandRecorder;
    friend class PipelineState;

    typedef std::pair<uint16, uint16> SetSlotPair; // first is set, second is binding
    typedef std::map<std::string, SetSlotPair> SetSlotMap; // mapping Resource Name to Slot

    ShaderType mType;
    Common::UniquePtr<glslang::TShader> mShaderGlslang;
    Common::UniquePtr<glslang::TProgram> mProgramGlslang;
    std::vector<uint32> mShaderSpv;
    SetSlotMap mResourceSlotMap;
    VkShaderModule mShader;
    VkPipelineShaderStageCreateInfo mStageInfo;

    bool GetIODesc();
    void ParseResourceSlots();

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);

    bool Disassemble(bool html, std::string& output) override;
    int GetResourceSlotByName(const char* name) override;
};

} // namespace Renderer
} // namespace NFE
