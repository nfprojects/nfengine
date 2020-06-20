/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan Shader definitions
 */

#include "PCH.hpp"

#include "Shader.hpp"
#include "Defines.hpp"
#include "Device.hpp"
#include "Internal/Debugger.hpp"
#include "Engine/Common/FileSystem/File.hpp"

#include <cstring>
#include <cctype>


namespace NFE {
namespace Renderer {

namespace {

const int DEFAULT_VERSION = 460;
const std::string SHADER_HEADER_START = "#version 460\n\
#extension GL_ARB_separate_shader_objects: enable\n\
#extension GL_ARB_shading_language_420pack: enable\n\
#extension GL_ARB_shader_image_load_store: enable\n";
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
            NFE_LOG_ERROR("Shader code or path must be suplied");
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
    case ShaderType::Compute:
        lang = EShLangCompute;
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

    // set environment
    mShaderGlslang->setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, DEFAULT_VERSION);
    mShaderGlslang->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
    mShaderGlslang->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_1);

    // input shader strings
    const char * shaderStrs[] = { shaderHead.c_str(), code };
    mShaderGlslang->setStrings(shaderStrs, 2);
    mShaderGlslang->setEntryPoint("main");

    // parse to glslang's AST
    EShMessages msg = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules);
    if (!mShaderGlslang->parse(&glslang::DefaultTBuiltInResource, DEFAULT_VERSION, ENoProfile, false, false, msg))
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
    CHECK_VKRESULT(result, "Failed to create Shader module");

    Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mShader), VK_OBJECT_TYPE_SHADER_MODULE, desc.path);

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
    // Disassemble the shader, to provide parsing source for slot extraction
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
    // TODO get rid of std::vector and std::string

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

        const Common::String tokenName(tokens[1].substr(openBracketPos + 1, closeBracketPos - openBracketPos - 1).c_str());
        SetSlotMap::Iterator it;
        if (tokens[2] == "DescriptorSet" || tokens[2] == "Binding")
        {
            it = mResourceSlotMap.Find(tokenName);
            if (it == mResourceSlotMap.end())
            {
                it = mResourceSlotMap.Insert(tokenName, std::make_pair(static_cast<uint16>(0), static_cast<uint16>(0))).iterator;
            }
        }
        else
            continue;

        if (tokens[2] == "DescriptorSet")
        {
            NFE_LOG_DEBUG("Found resource %s with DescriptorSet = %s", tokens[1].c_str(), tokens[3].c_str());
            it->second.first = static_cast<uint16>(std::atoi(tokens[3].c_str()));
        }

        if (tokens[2] == "Binding")
        {
            NFE_LOG_DEBUG("Found resource %s with Binding = %s", tokens[1].c_str(), tokens[3].c_str());
            it->second.second = static_cast<uint16>(std::atoi(tokens[3].c_str()));
        }
    }

    return;
}

int Shader::GetResourceSlotByName(const char* name)
{
    auto it = mResourceSlotMap.Find(Common::String(name));
    if (it == mResourceSlotMap.End())
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
