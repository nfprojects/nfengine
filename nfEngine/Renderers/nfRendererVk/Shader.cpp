/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan Shader definitions
 */

#include "PCH.hpp"

#include "Shader.hpp"
#include "Defines.hpp"
#include "Device.hpp"
#include "nfCommon/FileSystem/File.hpp"
#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Containers/String.hpp"
#include "nfCommon/Utils/StringUtils.hpp"

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/disassemble.h>

#include <cstring>
#include <cctype>

namespace NFE {
namespace Renderer {

namespace {

// default limits for glslang shader resources
// taken from glslang standalone implementation
// TODO These should be taken from VkPhysicalDevice's limits.
//      Reconsider doing this properly.
const TBuiltInResource DEFAULT_RESOURCE = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};

const int DEFAULT_VERSION = 110;
const Common::String SHADER_HEADER_START = "#version 450\n\
#extension GL_ARB_separate_shader_objects: enable\n\
#extension GL_ARB_shading_language_420pack: enable\n";
const Common::String DEFINE_STR = "#define ";
const Common::String SHADER_HEADER_TAIL = "\0";

} // namespace


Shader::Shader()
    : mType(ShaderType::Unknown)
    , mShaderSpv()
    , mResourceSlotMap()
    , mShader(VK_NULL_HANDLE)
    , mStageInfo()
{}

Shader::~Shader()
{
    if (mShader != VK_NULL_HANDLE)
        vkDestroyShaderModule(gDevice->GetDevice(), mShader, nullptr);
}

bool Shader::Init(const ShaderDesc& desc)
{
    mType = desc.type;

    Common::DynArray<char> str;
    uint32 shaderSize = 0;
    const char* code = nullptr;

    if (desc.code == nullptr)
    {
        if (desc.path == nullptr)
        {
            NFE_LOG_ERROR("Shader code or path must be suplied");
            return false;
        }

        Common::File file(Common::StringView(desc.path), Common::AccessMode::Read);
        shaderSize = static_cast<uint32>(file.GetSize());
        str.Resize(shaderSize + 1);

        if (file.Read(str.Data(), shaderSize) != shaderSize)
            return false;
        str[shaderSize] = '\0';
        code = str.Data();
    }
    else
    {
        code = desc.code;
        shaderSize = static_cast<uint32>(strlen(code));
    }

    // construct a shader string containing all the macros
    Common::String shaderHead = SHADER_HEADER_START;
    if (desc.macrosNum > 0)
    {
        for (unsigned int i = 0; i < desc.macrosNum; ++i)
            shaderHead += DEFINE_STR + desc.macros[i].name + ' ' + desc.macros[i].value + '\n';
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
    default:
        NFE_LOG_ERROR("Incorrect shader type provided");
        return false;
    }

    // create and parse shader
    mShaderGlslang.Reset(new glslang::TShader(lang));
    if (!mShaderGlslang)
    {
        NFE_LOG_ERROR("Memory allocation failed");
        return false;
    }
    const char * shaderStrs[] = { shaderHead.Str(), code };
    mShaderGlslang->setStrings(shaderStrs, 2);
    mShaderGlslang->setEntryPoint("main");

    // TODO we might want to enable includes, so this includer is useless for later on
    glslang::TShader::ForbidInclude includer;
    EShMessages msg = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules);
    if (!mShaderGlslang->parse(&DEFAULT_RESOURCE, DEFAULT_VERSION, ENoProfile, false, false, msg, includer))
    {
        NFE_LOG_ERROR("Failed to parse shader file %s:\n%s", desc.path, mShaderGlslang->getInfoLog());
        return false;
    }

    // create temporary TProgram to extract an intermediate SPIR-V
    mProgramGlslang.Reset(new glslang::TProgram());
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


    glslang::TIntermediate* progInt = mProgramGlslang->getIntermediate(lang);
    if (!progInt)
    {
        NFE_LOG_ERROR("Unable to extract shader intermediate");
        return false;
    }

    spv::SpvBuildLogger spvLogger;
    glslang::GlslangToSpv(*progInt, mShaderSpv, &spvLogger);

    ParseResourceSlots();

    // now we have spirv representation of shader, provide it to Vulkan
    VkShaderModuleCreateInfo shaderInfo;
    VK_ZERO_MEMORY(shaderInfo);
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = mShaderSpv.size() * sizeof(uint32);
    shaderInfo.pCode = mShaderSpv.data();
    VkResult result = vkCreateShaderModule(gDevice->GetDevice(), &shaderInfo, nullptr, &mShader);
    VK_RETURN_FALSE_IF_FAILED(result, "Failed to create Shader module");

    VK_ZERO_MEMORY(mStageInfo);
    mStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    mStageInfo.stage = TranslateShaderTypeToVkShaderStage(desc.type);
    mStageInfo.module = mShader;
    mStageInfo.pName = "main";

    NFE_LOG_SUCCESS("Shader '%s' compiled successfully", desc.path);
    return true;
}

bool Shader::Disassemble(bool html, Common::String& output)
{
    NFE_UNUSED(html); // TODO

    std::stringstream ss;
    spv::Disassemble(ss, mShaderSpv);
    output = ss.str().c_str();

    return true;
}

bool Shader::GetIODesc()
{
    // TODO
    return false;
}

void Shader::ParseResourceSlots()
{
    std::stringstream disasm;
    spv::Disassemble(disasm, mShaderSpv);
    std::string disasmStr = disasm.str();

    Common::StringView shaderDisassembly(disasmStr.c_str(), static_cast<uint32>(disasmStr.size()));
    Common::DynArray<Common::StringView> names;
    Common::DynArray<Common::StringView> decorates;

    {
        Common::DynArray<Common::StringView> tokens = Common::Split(shaderDisassembly);

        const Common::StringView nameToken("Name");
        const Common::StringView decorateToken("Decorate");

        for (uint32 i = 0; i < tokens.Size(); ++i)
        {
            uint32 namePos = tokens[i].FindFirst(nameToken);
            uint32 decoratePos = tokens[i].FindFirst(decorateToken);

            if (namePos != Common::StringView::END())
                names.PushBack(tokens[i].Range(namePos, tokens[i].Length() - namePos));
            else if (decoratePos != Common::StringView::END())
                decorates.PushBack(tokens[i].Range(decoratePos, tokens[i].Length() - decoratePos));
        }
    }

    for (uint32 i = 0; i < decorates.Size(); ++i)
    {
        Common::DynArray<Common::StringView> subtokens = Common::Split(decorates[i], ' ');

        // SPIR-V assumes following order:
        //   [0] - "Decorate" keyword
        //   [1] - "XX(<name>)" (Resource SPIR-V identifier & name)
        //   [2] - DecoratorType (ex. DescriptorSet, Binding, Location)
        //   [3] - Decoration value (ex. Binding number for Binding DecoratorType)
        // Different amount than 4 tokens means we shouldn't care about it
        if (subtokens.Size() != 4)
            continue;

        uint32 openBracketPos = subtokens[1].FindFirst('(');
        uint32 closeBracketPos = subtokens[1].FindLast(')');

        Common::StringView tokenName = subtokens[1].Range(openBracketPos + 1, closeBracketPos - openBracketPos - 1);
        SetBindingMap::Iterator it;
        if (subtokens[2] == "DescriptorSet" || subtokens[2] == "Binding")
        {
            it = mResourceSlotMap.Find(tokenName);
            if (it == mResourceSlotMap.end())
            {
                SetBindingPair emptyPair;
                emptyPair.total = 0;
                it = mResourceSlotMap.Insert(tokenName, emptyPair).iterator;
            }
        }
        else
            continue;

        if (subtokens[2] == "DescriptorSet")
        {
            it->second.pair.set = static_cast<uint16>(std::atoi(subtokens[3].Data()));
            NFE_LOG_DEBUG("Found resource %s with DescriptorSet = %hu", Common::String(tokenName).Str(), it->second.pair.set);
        }

        if (subtokens[2] == "Binding")
        {
            it->second.pair.binding = static_cast<uint16>(std::atoi(subtokens[3].Data()));
            NFE_LOG_DEBUG("Found resource %s with Binding = %hu", Common::String(tokenName).Str(), it->second.pair.binding);
        }
    }

    return;
}

int Shader::GetResourceSlotByName(const char* name)
{
    auto it = mResourceSlotMap.Find(name);
    if (it == mResourceSlotMap.End())
        return -1;

    return it->second.total;
}

} // namespace Renderer
} // namespace NFE
