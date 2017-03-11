/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of Multishader utility
 */

#pragma once

#include "PCH.hpp"
#include "Multishader.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/FileSystem/File.hpp"
#include "../rapidjson/include/rapidjson/document.h"
#include "../rapidjson/include/rapidjson/filestream.h"

namespace NFE {
namespace Resource {

namespace {

const std::string gShadersRoot = "nfEngine/Shaders/";

} // namespace

using namespace Renderer;

Multishader::Multishader()
{
    mType = ShaderType::Unknown;
}

void Multishader::OnUnload()
{
    mSubShaders.clear();
}

bool Multishader::OnLoad()
{
    LOG_INFO("Loading multishader '%s'...", mName);

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
        LOG_ERROR("Failed to parse multishader file '%s'...", mName);
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

    /// read macros
    if (document.HasMember("macros") && document["macros"].IsArray())
    {
        const rapidjson::Value& macrosNode = document["macros"];

        for (rapidjson::SizeType i = 0; i < macrosNode.Size(); ++i)
        {
            const auto& macroNode = macrosNode[i];

            std::string macroName = macroNode["name"].GetString();

            MultishaderMacro macro;
            macro.minValue = macroNode["min"].GetInt();
            macro.maxValue = macroNode["max"].GetInt();
            if (macroNode.HasMember("default"))
                macro.defaultValue = macroNode["default"].GetInt();
            else
                macro.defaultValue = macro.minValue;

            if (macro.minValue > macro.maxValue)
            {
                LOG_ERROR("Invalid values ranges for macro: '%s'", macroName.c_str());
                return false;
            }

            mMacroNames.push_back(macroName);
            mMacros.push_back(macro);
        }
    }

    // TODO: load shader source code and get its modification date

    bool loadedSuccessfully = false;
    if (mMacros.size() > 0)
    {
        std::unique_ptr<int[]> macroValues(new int[mMacros.size()]);

        size_t totalCombinations = 1;
        for (size_t i = 0; i < mMacros.size(); ++i)
        {
            const MultishaderMacro& macro = mMacros[i];
            totalCombinations *= (macro.maxValue - macro.minValue + 1);
            macroValues[i] = macro.minValue;
        }

        /// generate and load all multishader combinations
        for (size_t i = 0; i < totalCombinations; ++i)
        {
            loadedSuccessfully &= LoadSubshader(macroValues.get());

            macroValues[0]++;
            for (size_t j = 0; j < mMacros.size() - 1; ++j)
            {
                if (macroValues[j] > mMacros[j].maxValue)
                {
                    macroValues[j] = mMacros[j].minValue;
                    (macroValues[j + 1])++;
                }
            }
        }
    }
    else
    {
        loadedSuccessfully = LoadSubshader(nullptr);
    }

    return loadedSuccessfully;
}

bool Multishader::LoadSubshader(int* macroValues)
{
    // TODO: shader bytecode cache

    std::vector<ShaderMacro> macros;
    std::vector<std::string> valuesStrings;
    macros.reserve(mMacros.size());
    valuesStrings.reserve(mMacros.size());

    std::string shaderLanguage = "HLSL5"; // TODO: move to IDevice or HighLevelRenderer
    std::string shaderExt = ".hlsl"; // TODO: move to IDevice or HighLevelRenderer
    std::string shaderPath = gShadersRoot + shaderLanguage + '/' + mName + shaderExt;

    for (size_t i = 0; i < mMacros.size(); ++i)
    {
        // allocate all values on heap
        valuesStrings.push_back(std::to_string(macroValues[i]));

        ShaderMacro shaderMacro;
        shaderMacro.name = mMacroNames[i].c_str();
        shaderMacro.value = valuesStrings.back().c_str();
        macros.push_back(shaderMacro);
    }

    ShaderDesc shaderDesc;
    shaderDesc.path = shaderPath.c_str();
    shaderDesc.type = mType;
    shaderDesc.macros = macros.data();
    shaderDesc.macrosNum = macros.size();

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    mSubShaders.emplace_back(renderer->GetDevice()->CreateShader(shaderDesc));
    return mSubShaders.back() != nullptr;
}

size_t Multishader::GetMacrosNumber() const
{
    return mMacros.size();
}

int Multishader::GetMacroByName(const char* name) const
{
    for (size_t i = 0; i < mMacros.size(); ++i)
    {
        if (mMacroNames[i] == name)
            return static_cast<int>(i);
    }

    return -1;
}

const Renderer::ShaderPtr& Multishader::GetShader(int* values) const
{
    int subshaderId = 0;
    int multiplier = 1;

    // calculate subshader index based on macro values
    for (size_t i = 0; i < mMacros.size(); ++i)
    {
        const MultishaderMacro& macro = mMacros[i];
        int val = values[i];

        // check if macro value is in range
        if (val < macro.minValue || val > macro.maxValue)
            val = macro.defaultValue;

        val -= macro.minValue;
        subshaderId += val * multiplier;
        multiplier *= (macro.maxValue - macro.minValue + 1);
    }

    return mSubShaders[subshaderId];
}

} // namespace Renderer
} // namespace NFE
