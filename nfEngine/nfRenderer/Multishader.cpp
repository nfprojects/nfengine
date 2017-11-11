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
#include "nfCommon/Utils/StringUtils.hpp"

#include "../rapidjson/include/rapidjson/document.h"
#include "../rapidjson/include/rapidjson/filestream.h"

namespace NFE {
namespace Renderer {

namespace {

const Common::String gShadersRoot = "nfEngine/Shaders/";

} // namespace

Multishader::Multishader()
{
    mType = ShaderType::Unknown;
}

void Multishader::OnUnload()
{
    mSubShaders.Clear();
}

bool Multishader::OnLoad()
{
    NFE_LOG_INFO("Loading multishader '%s'...", mName.Str());

    // TODO: check JSON file modification date

    /// read multishader JSON file
    Common::DynArray<char> str;
    {
        const Common::String path = gShadersRoot + mName + ".json";
        Common::File file(path, Common::AccessMode::Read);
        size_t fileSize = static_cast<size_t>(file.GetSize());
        str.Resize(fileSize + 1);

        if (file.Read(str.Data(), fileSize) != fileSize)
            return false;
        str[fileSize] = '\0';
    }

    /// parse JSON
    rapidjson::Document document;
    if (document.Parse<0>(str.Data()).HasParseError())
    {
        NFE_LOG_ERROR("Failed to parse multishader file '%s'...", mName);
        return false;
    }

    /// extract shader type
    const rapidjson::Value& typeNode = document["type"];
    std::string shaderTypeStr = typeNode.GetString();
    std::transform(shaderTypeStr.begin(), shaderTypeStr.end(), shaderTypeStr.begin(), [](const char c) -> char
    {
        if (c >= 'A' && c <= 'Z')
            return c + ('Z' - 'A');
        else
            return c;
    });

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
        NFE_LOG_ERROR("Unknown shader type: '%s'", typeNode.GetString());
        return false;
    }

    /// read macros
    if (document.HasMember("macros") && document["macros"].IsArray())
    {
        const rapidjson::Value& macrosNode = document["macros"];

        for (rapidjson::SizeType i = 0; i < macrosNode.Size(); ++i)
        {
            const auto& macroNode = macrosNode[i];
            const Common::String macroName = macroNode["name"].GetString();

            MultishaderMacro macro;
            macro.minValue = macroNode["min"].GetInt();
            macro.maxValue = macroNode["max"].GetInt();
            if (macroNode.HasMember("default"))
                macro.defaultValue = macroNode["default"].GetInt();
            else
                macro.defaultValue = macro.minValue;

            if (macro.minValue > macro.maxValue)
            {
                NFE_LOG_ERROR("Invalid values ranges for macro: '%s'", macroName.Str());
                return false;
            }

            mMacroNames.PushBack(macroName);
            mMacros.PushBack(macro);
        }
    }

    // TODO: load shader source code and get its modification date

    bool loadedSuccessfully = false;
    if (!mMacros.Empty())
    {
        std::unique_ptr<int[]> macroValues(new int[mMacros.Size()]);

        uint32 totalCombinations = 1;
        for (uint32 i = 0; i < mMacros.Size(); ++i)
        {
            const MultishaderMacro& macro = mMacros[i];
            totalCombinations *= (macro.maxValue - macro.minValue + 1);
            macroValues[i] = macro.minValue;
        }

        /// generate and load all multishader combinations
        for (uint32 i = 0; i < totalCombinations; ++i)
        {
            loadedSuccessfully &= LoadSubshader(macroValues.get());

            macroValues[0]++;
            for (uint32 j = 0; j < mMacros.Size() - 1; ++j)
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

    Common::DynArray<ShaderMacro> macros;
    Common::DynArray<Common::String> valuesStrings;
    macros.Reserve(mMacros.Size());
    valuesStrings.Reserve(mMacros.Size());

    const Common::String shaderLanguage = "HLSL5"; // TODO: move to IDevice or HighLevelRenderer
    const Common::String shaderExt = ".hlsl"; // TODO: move to IDevice or HighLevelRenderer
    const Common::String shaderPath = gShadersRoot + shaderLanguage + '/' + mName + shaderExt;

    for (uint32 i = 0; i < mMacros.Size(); ++i)
    {
        // allocate all values on heap
        valuesStrings.PushBack(Common::ToString(macroValues[i]));

        ShaderMacro shaderMacro;
        shaderMacro.name = mMacroNames[i].Str();
        shaderMacro.value = valuesStrings.Back().Str();
        macros.PushBack(shaderMacro);
    }

    ShaderDesc shaderDesc;
    shaderDesc.path = shaderPath.Str();
    shaderDesc.type = mType;
    shaderDesc.macros = macros.Data();
    shaderDesc.macrosNum = static_cast<uint32>(macros.Size());

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    mSubShaders.EmplaceBack(renderer->GetDevice()->CreateShader(shaderDesc));
    return mSubShaders.Back() != nullptr;
}

uint32 Multishader::GetMacrosNumber() const
{
    return mMacros.Size();
}

int Multishader::GetMacroByName(const char* name) const
{
    for (uint32 i = 0; i < mMacros.Size(); ++i)
    {
        if (mMacroNames[i] == name)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

const ShaderPtr& Multishader::GetShader(int* values) const
{
    int subshaderId = 0;
    int multiplier = 1;

    // calculate subshader index based on macro values
    for (uint32 i = 0; i < mMacros.Size(); ++i)
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
