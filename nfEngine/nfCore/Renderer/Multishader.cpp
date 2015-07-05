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

    /// read macros
    if (document.HasMember("macros") && document["macros"].IsArray())
    {
        const rapidjson::Value& macrosNode = document["macros"];

        for (rapidjson::SizeType i = 0; i < macrosNode.Size(); ++i)
        {
            const auto& macroNode = macrosNode[i];
            MultishaderMacro macro;
            macro.name = macroNode["name"].GetString();
            macro.minValue = macroNode["min"].GetInt();
            macro.maxValue = macroNode["max"].GetInt();
            if (macroNode.HasMember("default"))
                macro.defaultValue = macroNode["default"].GetInt();
            else
                macro.defaultValue = macro.minValue;

            if (macro.minValue > macro.maxValue)
            {
                LOG_ERROR("Invalid values ranges for macro: '%s'", macro.name.c_str());
                return false;
            }

            mMacros.push_back(macro);
        }
    }

    // TODO: load shader source code and get modif

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

        /// generate and load all mutlishader combinations
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
    std::vector<ShaderMacro> macros;
    std::vector<std::string> valuesStrings;
    macros.reserve(mMacros.size());
    valuesStrings.reserve(mMacros.size());

    std::string rendererName = "D3D11"; // TODO: move to IDevice or HighLevelRenderer
    std::string shaderExt = ".hlsl"; // TODO: move to IDevice or HighLevelRenderer
    std::string shaderPath = gShadersRoot + rendererName + '/' + mName + shaderExt;

    for (size_t i = 0; i < mMacros.size(); ++i)
    {
        const MultishaderMacro& macro = mMacros[i];

        // allocate all values on heap
        valuesStrings.push_back(std::to_string(macroValues[i]));

        ShaderMacro shaderMacro;
        shaderMacro.name = macro.name.c_str();
        shaderMacro.value = valuesStrings.back().c_str();
        macros.push_back(shaderMacro);
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

size_t Multishader::GetMacrosNumber() const
{
    return mMacros.size();
}

int Multishader::GetMacroByName(const char* name) const
{
    for (size_t i = 0; i < mMacros.size(); ++i)
    {
        if (mMacros[i].name == name)
            return static_cast<int>(i);
    }

    return -1;
}

IShader* Multishader::GetShader(int* values) const
{


    return mSubShaders[0].get();
}

} // namespace Renderer
} // namespace NFE
