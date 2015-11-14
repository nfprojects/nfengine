/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of MultiShaderProgram utility
 */

#pragma once

#include "PCH.hpp"
#include "MultiShaderProgram.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/File.hpp"
#include "../rapidjson/include/rapidjson/document.h"
#include "../rapidjson/include/rapidjson/filestream.h"

namespace NFE {
namespace Resource {

namespace {

const std::string gShadersProgramRoot = "nfEngine/Shaders/Programs/";

const char* gShaderNameStrings[] =
{
    "vertexShader",
    "hullShader",
    "domainShader",
    "geometryShader",
    "pixelShader"
};

void ShaderResourceDeleter(Multishader* multishader)
{
    if (multishader)
        multishader->DelRef();
}

} // namespace

using namespace Renderer;


MultiShaderProgram::MultiShaderProgram()
    : mShaderResources
        {
            ShaderResourcePtr(nullptr, ShaderResourceDeleter),
            ShaderResourcePtr(nullptr, ShaderResourceDeleter),
            ShaderResourcePtr(nullptr, ShaderResourceDeleter),
            ShaderResourcePtr(nullptr, ShaderResourceDeleter),
            ShaderResourcePtr(nullptr, ShaderResourceDeleter)
        }
{
}

bool MultiShaderProgram::Load(const char* name)
{
    ResManager* rm = Engine::GetInstance()->GetResManager();
    LOG_INFO("Loading multi shader program '%s'...", name);

    /// read JSON file
    std::vector<char> str;
    {
        Common::File file(gShadersProgramRoot + name + ".json", Common::AccessMode::Read);
        size_t fileSize = static_cast<size_t>(file.GetSize());
        str.resize(fileSize + 1);

        if (file.Read(str.data(), fileSize) != fileSize)
            return false;
        str[fileSize] = '\0';
    }

    // parse JSON

    rapidjson::Document document;
    if (document.Parse<0>(str.data()).HasParseError())
    {
        LOG_ERROR("Failed to parse multi shader program file '%s'...", name);
        return false;
    }

    // read shader names
    for (int i = 0; i < NFE_SHADER_TYPES_NUM; ++i)
    {
        if (document.HasMember(gShaderNameStrings[i]))
        {
            const char* shaderName = document[gShaderNameStrings[i]].GetString();
            Multishader* resource = static_cast<Multishader*>(rm->GetResource(shaderName,
                                                                              ResourceType::Shader));
            mShaderResources[i] = ShaderResourcePtr(resource, ShaderResourceDeleter);
            mShaderResources[i]->AddRef();
        }
    }

    // read macros
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

    return GenerateShaderPrograms();
}

bool MultiShaderProgram::GenerateShaderPrograms()
{
    ResManager* rm = Engine::GetInstance()->GetResManager();

    // wait for shader resources and build macro mapping table
    for (size_t i = 0; i < NFE_SHADER_TYPES_NUM; ++i)
    {
        Multishader* shader = mShaderResources[i].get();
        if (shader)
        {
            mShaderMacroMapping[i].resize(mMacros.size());
            rm->WaitForResource(shader);

            for (size_t j = 0; j < mMacros.size(); ++j)
                mShaderMacroMapping[i][j] = shader->GetMacroByName(mMacroNames[j].c_str());
        }
    }

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

        // generate and load all shader program combinations
        for (size_t i = 0; i < totalCombinations; ++i)
        {
            loadedSuccessfully &= LoadSubShaderProgram(macroValues.get());

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
        loadedSuccessfully = LoadSubShaderProgram(nullptr);
    }

    return loadedSuccessfully;
}

bool MultiShaderProgram::LoadSubShaderProgram(int* macroValues)
{
    ShaderProgramDesc desc;

    for (size_t i = 0; i < NFE_SHADER_TYPES_NUM; ++i)
    {
        if (!mShaderResources[i])
            continue;

        int shaderMacros[16] = { -1 };
        for (size_t j = 0; j < mMacros.size(); ++j)
        {
            int dest = mShaderMacroMapping[i][j];
            if (dest >= 0)
                shaderMacros[dest] = macroValues[j];
        }

        IShader* shader = mShaderResources[i]->GetShader(shaderMacros);
        ShaderType type = static_cast<ShaderType>(i);

        switch (type)
        {
        case ShaderType::Vertex:
            desc.vertexShader = shader;
            break;
        case ShaderType::Hull:
            desc.hullShader = shader;
            break;
        case ShaderType::Domain:
            desc.domainShader = shader;
            break;
        case ShaderType::Geometry:
            desc.geometryShader = shader;
            break;
        case ShaderType::Pixel:
            desc.pixelShader = shader;
            break;
        }
    }

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    std::unique_ptr<IShaderProgram> shaderProgram(renderer->GetDevice()->CreateShaderProgram(desc));
    mSubPrograms.push_back(std::move(shaderProgram));

    return shaderProgram.get() != nullptr;
}

size_t MultiShaderProgram::GetMacrosNumber() const
{
    return mMacros.size();
}

int MultiShaderProgram::GetMacroByName(const char* name) const
{
    for (size_t i = 0; i < mMacros.size(); ++i)
    {
        if (mMacroNames[i] == name)
            return static_cast<int>(i);
    }

    return -1;
}

int MultiShaderProgram::CalculateShaderProgramID(int* macroValues) const
{
    int subshaderId = 0;
    int multiplier = 1;

    // calculate subshader index based on macro values
    for (size_t i = 0; i < mMacros.size(); ++i)
    {
        const MultishaderMacro& macro = mMacros[i];
        int val = macroValues[i];

        // check if macro value is in range
        if (val < macro.minValue || val > macro.maxValue)
            val = macro.defaultValue;

        val -= macro.minValue;
        subshaderId += val * multiplier;
        multiplier *= (macro.maxValue - macro.minValue + 1);
    }

    return subshaderId;
}

IShaderProgram* MultiShaderProgram::GetShaderProgram(int* values) const
{
    int subshaderId = CalculateShaderProgramID(values);
    return mSubPrograms[subshaderId].get();
}

IShader* MultiShaderProgram::GetShader(Renderer::ShaderType type, int* values) const
{
    int typeId = static_cast<int>(type);

    int macroValues[16] = { 0 };

    for (size_t i = 0; i < mMacros.size(); ++i)
    {
        int mapping = mShaderMacroMapping[typeId][i];
        if (mapping >= 0)
            macroValues[mapping] = values[i];
    }

    return mShaderResources[typeId]->GetShader(macroValues);
}

} // namespace Renderer
} // namespace NFE
