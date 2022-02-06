/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Vulkan Shader definitions
 */

#include "PCH.hpp"

#include "Shader.hpp"
#include "Defines.hpp"
#include "Device.hpp"

#include "Internal/Debugger.hpp"
#include "Internal/ShaderIncluder.hpp"

#include <Engine/Common/FileSystem/File.hpp>
#include <Engine/Common/FileSystem/FileSystem.hpp>

#include <vector>
#include <cstring>
#include <cctype>


namespace NFE {
namespace Renderer {

namespace {

const int DEFAULT_VERSION = 460;
const Common::String SHADER_HEADER_START("#version 460\n\
#extension GL_ARB_separate_shader_objects: enable\n\
#extension GL_ARB_shading_language_420pack: enable\n\
#extension GL_ARB_shader_image_load_store: enable\n");
const Common::String DEFINE_STR("#define ");
const Common::String SHADER_HEADER_TAIL("\0");

} // namespace


Shader::Shader()
    : mType(ShaderType::Unknown)
    , mShaderPath()
    , mShaderGlslang()
    , mProgramGlslang()
    , mStageInfo()
    , mSpvReflectModule()
{
}

Shader::~Shader()
{
}

bool Shader::Init(const ShaderDesc& desc)
{
    mType = desc.type;
    mShaderPath = desc.path;

    Common::DynArray<char> str;
    int32 shaderSize = 0;
    const char* code = nullptr;

    if (desc.code == nullptr)
    {
        if (desc.path == nullptr)
        {
            NFE_LOG_ERROR("Shader code or path must be suplied");
            return false;
        }

        Common::File file(desc.path, Common::AccessMode::Read);
        shaderSize = static_cast<int32>(file.GetSize());
        str.Resize(shaderSize + 1);

        if (file.Read(str.Data(), shaderSize) != shaderSize)
            return false;
        str[shaderSize] = '\0';
        code = str.Data();
    }
    else
    {
        code = desc.code;
        shaderSize = static_cast<int32>(strlen(code));
    }

    // construct a shader string containing all the macros
    Common::String shaderHead;
    if (desc.macrosNum > 0)
    {
        for (uint32 i = 0; i < desc.macrosNum; ++i)
        {
            shaderHead += DEFINE_STR + desc.macros[i].name + ' ' + desc.macros[i].value + '\n';
        }
    }

    // null-termination to help glslang
    shaderHead += SHADER_HEADER_TAIL;

    // determine shader's type
    EShLanguage lang;
    switch (desc.type)
    {
    case ShaderType::Vertex:
        lang = EShLangVertex;
        break;
    case ShaderType::Hull:
        lang = EShLangTessControl;
        break;
    case ShaderType::Domain:
        lang = EShLangTessEvaluation;
        break;
    case ShaderType::Geometry:
        lang = EShLangGeometry;
        break;
    case ShaderType::Pixel:
        lang = EShLangFragment;
        break;
    case ShaderType::Compute:
        lang = EShLangCompute;
        break;
    default:
        NFE_LOG_ERROR("Incorrect shader type provided");
        return false;
    }

    // create and parse shader
    mShaderGlslang = Common::MakeUniquePtr<glslang::TShader>(lang);
    if (!mShaderGlslang)
    {
        NFE_LOG_ERROR("Memory allocation failed");
        return false;
    }

    // set environment
    mShaderGlslang->setEnvInput(glslang::EShSourceHlsl, lang, glslang::EShClientVulkan, DEFAULT_VERSION);
    mShaderGlslang->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
    mShaderGlslang->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);

    // input shader strings
    const char* shaderStrs[] = { shaderHead.Str(), code };
    int shaderLengths[] = { static_cast<int>(shaderHead.Length()), shaderSize };
    const char* shaderNames[] = { "RendererVk_ShaderHead", desc.path };
    mShaderGlslang->setStringsWithLengthsAndNames(shaderStrs, shaderLengths, shaderNames, 2);
    mShaderGlslang->setEntryPoint("main");

    // parse to glslang's AST
    ShaderIncluder includer(desc.path);
    EShMessages msg = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgReadHlsl);
    if (!mShaderGlslang->parse(&glslang::DefaultTBuiltInResource, DEFAULT_VERSION, EProfile::ENoProfile, false, false, msg, includer))
    {
        NFE_LOG_ERROR("Failed to parse shader file %s:\n%s", desc.path, mShaderGlslang->getInfoLog());
        return false;
    }

    // create temporary TProgram to extract an intermediate SPIR-V
    mProgramGlslang = Common::MakeUniquePtr<glslang::TProgram>();
    if (!mProgramGlslang)
    {
        NFE_LOG_ERROR("Memory allocation failed");
        return false;
    }

    mProgramGlslang->addShader(mShaderGlslang.Get());
    if (!mProgramGlslang->link(msg))
    {
        NFE_LOG_ERROR("Failed to pre-link shader stage:\n%s", mProgramGlslang->getInfoLog());
        return false;
    }

    // TODO we should try avoiding this and use spirv-reflect instead.
    // Problem is, glslang loses names of descriptor sets when generating SPV bytecode.
    // Try to solve this somehow (maybe) or remove this comment if it's impossible.
    if (!mProgramGlslang->buildReflection())
    {
        NFE_LOG_ERROR("Failed to build reflection for shader program");
        return false;
    }

    glslang::TIntermediate* progInt = mProgramGlslang->getIntermediate(lang);
    if (!progInt)
    {
        NFE_LOG_ERROR("Unable to extract shader intermediate");
        return false;
    }

    spv::SpvBuildLogger spvLogger;
    std::vector<uint32> shaderSpv;
    glslang::GlslangToSpv(*progInt, shaderSpv, &spvLogger);

    SpvReflectResult spvResult = spvReflectCreateShaderModule2(
        SPV_REFLECT_MODULE_FLAG_NONE, shaderSpv.size() * sizeof(uint32), shaderSpv.data(), &mSpvReflectModule
    );
    CHECK_SPVREFLECTRESULT(spvResult, "Failed to create reflection of shader SPIR-V code");

    // Prepare shader stage info for later use
    // Shader Module is not created here - it will be modified when creating PSO in order to
    // remap descriptor sets
    VK_ZERO_MEMORY(mStageInfo);
    mStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    mStageInfo.stage = TranslateShaderTypeToVkShaderStage(desc.type);
    mStageInfo.pName = "main";

    NFE_LOG_SUCCESS("Shader '%s' compiled to SPV successfully", desc.path);
    return true;
}

bool Shader::Disassemble(bool html, Common::String& output)
{
    NFE_UNUSED(html); // TODO
    // Disassemble the shader, to provide parsing source for slot extraction

    uint32 spvCodeSize = spvReflectGetCodeSize(&mSpvReflectModule);
    std::vector<uint32> shaderSpv(spvCodeSize / sizeof(uint32));
    memcpy(shaderSpv.data(), spvReflectGetCode(&mSpvReflectModule), spvCodeSize);

    std::stringstream ss;
    spv::Disassemble(ss, shaderSpv);
    output = ss.str().c_str();
    return true;
}

int Shader::GetResourceSlotByName(const char* name)
{
    auto ExtractBindingSetSlots = [](const glslang::TObjectReflection& o) -> int
    {
        // encode slot/binding pair onto a single uint
        // 16 MSB are set slot, 16 LSB are binding slot in that set
        const glslang::TQualifier& qualifier = o.getType()->getQualifier();
        int result = (qualifier.layoutSet & 0xFFFF) << 16;
        result |= (qualifier.layoutBinding & 0xFFFF);
        return static_cast<int>(result);
    };

    for (int i = 0; i < mProgramGlslang->getNumUniformBlocks(); ++i)
    {
        const glslang::TObjectReflection& obj = mProgramGlslang->getUniformBlock(i);
        if (obj.name.compare(name) == 0)
            return ExtractBindingSetSlots(obj);
    }

    for (int i = 0; i < mProgramGlslang->getNumUniformVariables(); ++i)
    {
        const glslang::TObjectReflection& obj = mProgramGlslang->getUniform(i);
        if (obj.name.compare(name) == 0)
            return ExtractBindingSetSlots(obj);
    }

    return -1;
}

} // namespace Renderer
} // namespace NFE
