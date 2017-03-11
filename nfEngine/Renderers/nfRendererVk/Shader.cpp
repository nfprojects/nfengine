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

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/disassemble.h>

#include <cstring>
#include <cctype>

namespace NFE {
namespace Renderer {

namespace {

// default limits for glslang shader resources
// taken from glslang standalone implementation
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
const std::string SHADER_HEADER_START = "#version 450\n\
#extension GL_ARB_separate_shader_objects: enable\n\
#extension GL_ARB_shading_language_420pack: enable\n";
const std::string DEFINE_STR = "#define ";
const std::string SHADER_HEADER_TAIL = "\0";

} // namespace


Shader::Shader()
    : mType(ShaderType::Unknown)
    , mShader(VK_NULL_HANDLE)
{}

Shader::~Shader()
{
    if (mShader != VK_NULL_HANDLE)
        vkDestroyShaderModule(gDevice->GetDevice(), mShader, nullptr);
}

bool Shader::Init(const ShaderDesc& desc)
{
    mType = desc.type;

    std::vector<char> str;
    size_t shaderSize = 0;
    const char* code = nullptr;

    if (desc.code == nullptr)
    {
        if (desc.path == nullptr)
        {
            LOG_ERROR("Shader code or path must be suplied");
            return false;
        }

        using namespace Common;
        File file(desc.path, AccessMode::Read);
        shaderSize = static_cast<size_t>(file.GetSize());
        str.resize(shaderSize + 1);

        if (file.Read(str.data(), shaderSize) != shaderSize)
            return false;
        str[shaderSize] = '\0';
        code = str.data();
    }
    else
    {
        code = desc.code;
        shaderSize = strlen(code);
    }

    // construct a shader string containing all the macros
    std::string shaderHead = SHADER_HEADER_START;
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
        LOG_ERROR("Incorrect shader type provided");
        return false;
    }

    // create and parse shader
    mShaderGlslang.reset(new (std::nothrow) glslang::TShader(lang));
    const char * shaderStrs[] = { shaderHead.c_str(), code };
    mShaderGlslang->setStrings(shaderStrs, 2);
    mShaderGlslang->setEntryPoint("main");

    // TODO we might want to enable includes, so this includer is useless for later on
    glslang::TShader::ForbidInclude includer;
    EShMessages msg = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules);
    if (!mShaderGlslang->parse(&DEFAULT_RESOURCE, DEFAULT_VERSION, ENoProfile, false, false, msg, includer))
    {
        LOG_ERROR("Failed to parse shader file %s:\n%s", desc.path, mShaderGlslang->getInfoLog());
        return false;
    }

    // create temporary TProgram to extract an intermediate SPIR-V
    mProgramGlslang.reset(new (std::nothrow) glslang::TProgram());
    mProgramGlslang->addShader(mShaderGlslang.get());
    if (!mProgramGlslang->link(msg))
    {
        LOG_ERROR("Failed to pre-link shader stage:\n%s", mProgramGlslang->getInfoLog());
        return false;
    }

    glslang::TIntermediate* progInt = mProgramGlslang->getIntermediate(lang);
    if (!progInt)
    {
        LOG_ERROR("Unable to extract shader intermediate");
        return false;
    }

    std::string errorMessages;
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
    CHECK_VKRESULT(result, "Failed to create Shader module");

    VK_ZERO_MEMORY(mStageInfo);
    mStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    mStageInfo.stage = TranslateShaderTypeToVkShaderStage(desc.type);
    mStageInfo.module = mShader;
    mStageInfo.pName = "main";

    LOG_SUCCESS("Shader '%s' compiled successfully", desc.path);
    return true;
}

bool Shader::Disassemble(bool html, std::string& output)
{
    UNUSED(html); // TODO
    // Disassemble the shader, to provide parsing source for slot extraction
    std::stringstream ss;
    spv::Disassemble(ss, mShaderSpv);
    output = ss.str();
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

    std::string line;
    std::vector<std::string> names;
    std::vector<std::string> decorates;
    while (std::getline(disasm, line))
    {
        size_t namePos = line.find("Name");
        size_t decoratePos = line.find("Decorate");
        if (namePos == std::string::npos && decoratePos == std::string::npos)
            continue;

        // trim from starting whitespaces
        line.erase(line.begin(), std::find_if(line.begin(), line.end(),
            [](char c) -> bool { return !std::isspace(c); }));

        if (namePos == std::string::npos)
            decorates.push_back(line);
        else
            names.push_back(line);
    }

    // TODO decoration-name matching should be done by resource IDs
    for (auto& decoration: decorates)
    {
        // extract number which relates to provided decorate
        std::istringstream iss(decoration);
        std::vector<std::string> tokens;
        std::string token;
        while (std::getline(iss, token,' '))
            tokens.push_back(token);

        // SPIR-V assumes following order:
        //   [0] - "Decorate" keyword
        //   [1] - "XX(<name>)" (Resource SPIR-V identifier & name)
        //   [2] - DecoratorType (ex. DescriptorSet, Binding, Location)
        //   [3] - Decoration value (ex. Binding number for Binding DecoratorType)
        // Different amount than 4 tokens means we shouldn't care about it
        if (tokens.size() != 4)
            continue;

        // trim ID(name) to acquire these values
        size_t openBracketPos = tokens[1].find('(');
        size_t closeBracketPos = tokens[1].find(')', openBracketPos+1);

        std::string tokenName = tokens[1].substr(openBracketPos+1, closeBracketPos - openBracketPos - 1);
        SetSlotMap::iterator it;
        if (tokens[2] == "DescriptorSet" || tokens[2] == "Binding")
        {
            it = mResourceSlotMap.find(tokenName);
            if (it == mResourceSlotMap.end())
                it = std::get<0>(mResourceSlotMap.emplace(tokenName, std::make_pair(0, 0)));
        }
        else
            continue;

        if (tokens[2] == "DescriptorSet")
        {
            LOG_DEBUG("Found resource %s with DescriptorSet = %s", tokens[1].c_str(), tokens[3].c_str());
            it->second.first = static_cast<uint16>(std::atoi(tokens[3].c_str()));
        }

        if (tokens[2] == "Binding")
        {
            LOG_DEBUG("Found resource %s with Binding = %s", tokens[1].c_str(), tokens[3].c_str());
            it->second.second = static_cast<uint16>(std::atoi(tokens[3].c_str()));
        }
    }

    return;
}

int Shader::GetResourceSlotByName(const char* name)
{
    auto it = mResourceSlotMap.find(name);
    if (it == mResourceSlotMap.end())
        return -1;

    const SetSlotPair& pair = it->second;

    // encode slot/binding pair onto a single uint
    // 16 MSB are set, 16 LSB are binding point
    uint32 result = std::get<0>(pair);
    result <<= 16;
    result |= std::get<1>(pair);

    return static_cast<int>(result);
}

} // namespace Renderer
} // namespace NFE
