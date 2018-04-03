/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan Shader declarations
 */

#pragma once

#include "../RendererInterface/Shader.hpp"
#include "Defines.hpp"

#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Containers/HashMap.hpp"

// TODO remove
#include <vector>

namespace NFE {
namespace Renderer {

class Shader : public IShader
{
    friend class CommandRecorder;
    friend class PipelineState;

    typedef std::pair<uint16, uint16> SetSlotPair; // first is set, second is binding
    typedef Common::HashMap<Common::String, SetSlotPair> SetSlotMap; // mapping Resource Name to Slot

    ShaderType mType;
    Common::UniquePtr<glslang::TShader> mShaderGlslang;
    Common::UniquePtr<glslang::TProgram> mProgramGlslang;
    std::vector<uint32> mShaderSpv; // TODO remove
    SetSlotMap mResourceSlotMap;
    VkShaderModule mShader;
    VkPipelineShaderStageCreateInfo mStageInfo;

    bool GetIODesc();
    void ParseResourceSlots();

public:
    Shader();
    ~Shader();
    bool Init(const ShaderDesc& desc);

    bool Disassemble(bool html, Common::String& output) override;
    int GetResourceSlotByName(const char* name) override;
};

} // namespace Renderer
} // namespace NFE
