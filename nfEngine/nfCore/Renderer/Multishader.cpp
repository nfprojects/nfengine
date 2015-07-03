/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of Multishader utility
 */

#pragma once

#include "../PCH.hpp"
#include "Multishader.hpp"
#include "../Globals.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/File.hpp"
#include "../rapidjson/include/rapidjson/document.h"
#include "../rapidjson/include/rapidjson/filestream.h"

namespace NFE {
namespace Renderer {

namespace {

const std::string gShadersRoot = "nfEngine/Shaders/";

} // namespace


Multishader::Multishader()
{
    mType = ShaderType::Unknown;
}

bool Multishader::Load(const char* name)
{
    LOG_INFO("Loading multishader '%s'...", name);
    mName = name;

    // TODO: check JSON file modification date

    /// read multishader JSON file
    std::vector<char> str;
    {
        Common::File file(gShadersRoot + mName + ".json", Common::AccessMode::Read);
        size_t fileSize = static_cast<size_t>(file.GetSize());
        str.resize(fileSize + 1);

        if (file.Read(str.data(), fileSize) != fileSize)
            return false;
        str[fileSize] = '\0';
    }

    /// parse JSON
    rapidjson::Document document;
    if (document.Parse<0>(str.data()).HasParseError())
    {
        LOG_ERROR("Failed to parse multishader file '%s'...", name);
        return false;
    }

    /// extract shader type
    const rapidjson::Value& typeNode = document["type"];
    std::string shaderTypeStr = typeNode.GetString();
    std::transform(shaderTypeStr.begin(), shaderTypeStr.end(), shaderTypeStr.begin(), ::tolower);
    if (shaderTypeStr == "vertex")
        mType = ShaderType::Vertex;
    else if (shaderTypeStr == "hull")
        mType = ShaderType::Hull;
    else if(shaderTypeStr == "domain")
        mType = ShaderType::Domain;
    else if (shaderTypeStr == "geometry")
        mType = ShaderType::Geometry;
    else if(shaderTypeStr == "pixel")
        mType = ShaderType::Pixel;
    else
    {
        LOG_ERROR("Unknown shader type: '%s'", typeNode.GetString());
        return false;
    }

    /// read defines
    if (document.HasMember("defines") && document["defines"].IsArray())
    {
        const rapidjson::Value& definesNode = document["defines"];

        for (rapidjson::SizeType i = 0; i < definesNode.Size(); ++i)
        {
            const auto& defineNode = definesNode[i];
            MultishaderDefine define;
            define.name = defineNode["name"].GetString();
            define.minValue = defineNode["min"].GetInt();
            define.maxValue = defineNode["max"].GetInt();
            if (defineNode.HasMember("default"))
                define.defaultValue = defineNode["default"].GetInt();
            else
                define.defaultValue = define.minValue;

            if (define.minValue > define.maxValue)
            {
                LOG_ERROR("Invalid values ranges for define: '%s'", define.name.c_str());
                return false;
            }

            mDefines.push_back(define);
        }
    }

    // TODO: load shader source code and get modif

    std::unique_ptr<int[]> macroValues(new int[mDefines.size()]);

    size_t totalCombinations = 1;
    for (size_t i = 0; i < mDefines.size(); ++i)
    {
        const MultishaderDefine& define = mDefines[i];
        totalCombinations *= (define.maxValue - define.minValue + 1);
        macroValues[i] = define.minValue;
    }

    /// generate and load all mutlishader combinations
    bool loadedSuccessfully = false;
    for (size_t i = 0; i < totalCombinations; ++i)
    {
        loadedSuccessfully &= LoadSubshader(macroValues.get());

        macroValues[0]++;
        for (size_t j = 0; j < mDefines.size() - 1; ++j)
        {
            if (macroValues[j] > mDefines[j].maxValue)
            {
                macroValues[j] = mDefines[j].minValue;
                (macroValues[j + 1])++;
            }
        }
    }

    return true;
}

bool Multishader::LoadSubshader(int* defineValues)
{
    std::vector<ShaderMacro> macros;
    std::vector<std::string> valuesStrings;
    macros.reserve(mDefines.size());
    valuesStrings.reserve(mDefines.size());

    std::string rendererName = "D3D11"; // TODO: move to IDevice or HighLevelRenderer
    std::string shaderExt = ".hlsl"; // TODO: move to IDevice or HighLevelRenderer
    std::string shaderPath = gShadersRoot + rendererName + '/' + mName + shaderExt;

    for (size_t i = 0; i < mDefines.size(); ++i)
    {
        const MultishaderDefine& define = mDefines[i];

        // allocate all values on heap
        valuesStrings.push_back(std::to_string(defineValues[i]));

        ShaderMacro macro;
        macro.name = define.name.c_str();
        macro.value = valuesStrings.back().c_str();
        macros.push_back(macro);
    }

    ShaderDesc shaderDesc;
    shaderDesc.path = shaderPath.c_str();
    shaderDesc.type = mType;
    shaderDesc.macros = macros.data();
    shaderDesc.macrosNum = macros.size();

    std::unique_ptr<IShader> shader;
    shader.reset(gRenderer->GetDevice()->CreateShader(shaderDesc));
    mSubShaders.push_back(std::move(shader));

    return shader.get() != nullptr;
}

int Multishader::GetDefineByName(const char* name) const
{
    for (size_t i = 0; i < mDefines.size(); ++i)
    {
        if (mDefines[i].name == name)
            return i;
    }

    return -1;
}

IShader* Multishader::GetShader(int* defines, int* values, size_t definesNum) const
{
    return mSubShaders[0].get();
}

} // namespace Renderer
} // namespace NFE
