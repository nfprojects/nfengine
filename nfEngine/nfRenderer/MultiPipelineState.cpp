/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of MultiPipelineState utility
 */

#pragma once

#include "PCH.hpp"
#include "MultiPipelineState.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/FileSystem/File.hpp"
#include "nfCommon/Config/ConfigValue.hpp"
#include "nfCommon/Config/Config.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;

namespace {

const String gShadersSetsRoot = "nfEngine/Shaders/Sets/";

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


MultiPipelineState::MultiPipelineState()
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

bool MultiPipelineState::Load(const char* name)
{
    mName = name;

    NFE_LOG_INFO("Loading multi shader set '%s'...", name);

    /// read config file
    DynArray<char> str;
    {
        const String path = gShadersSetsRoot + name + ".cfg";
        File file(path, AccessMode::Read);
        size_t fileSize = static_cast<size_t>(file.GetSize());
        str.Resize(fileSize + 1);

        if (file.Read(str.Data(), fileSize) != fileSize)
            return false;
        str[fileSize] = '\0';
    }

    /// parse config file
    Config config;
    if (!config.ParseInPlace(str.Data()))
    {
        NFE_LOG_ERROR("Failed to parse shader set file '%s'...", name);
        return false;
    }

    ConfigGenericValue root(&config);

    // read shader names
    for (int i = 0; i < NFE_GRAPHICS_SHADER_TYPES_NUM; ++i)
    {
        ConfigGenericValue node = root[gShaderNameStrings[i]];
        if (node.IsString())
        {
            const char* shaderName = node.GetString();
            Multishader* resource = static_cast<Multishader*>(rm->GetResource(shaderName, ResourceType::Shader));
            mShaderResources[i] = ShaderResourcePtr(resource, ShaderResourceDeleter);
            mShaderResources[i]->AddRef();
        }
    }

    // read macros
    if (root.HasMember("macros") && root["macros"].IsArray())
    {
        ConfigGenericValue macrosNode = root["macros"];
        int macrosNum = static_cast<int>(macrosNode.GetSize());

        for (int i = 0; i < macrosNum; ++i)
        {
            ConfigGenericValue macroNode = macrosNode[i];
            if (!macroNode.IsObject())
            {
                NFE_LOG_ERROR("'macros' array must contain objects");
                return false;
            }

            const char* macroName = macroNode["name"].GetString();

            MultishaderMacro macro;
            macro.minValue = macroNode["min"].Get<int32>();
            macro.maxValue = macroNode["max"].Get<int32>();
            if (macroNode.HasMember("default"))
                macro.defaultValue = macroNode["default"].Get<int32>();
            else
                macro.defaultValue = macro.minValue;

            if (macro.minValue > macro.maxValue)
            {
                NFE_LOG_ERROR("Invalid values ranges for macro: '%s'", macroName);
                return false;
            }

            mMacroNames.PushBack(macroName);
            mMacros.PushBack(macro);
        }
    }

    GenerateShaderSets();
    return true;
}

void MultiPipelineState::GenerateShaderSets()
{
    ResManager* rm = Engine::GetInstance()->GetResManager();

    // wait for shader resources and build macro mapping table
    for (size_t i = 0; i < NFE_GRAPHICS_SHADER_TYPES_NUM; ++i)
    {
        Multishader* shader = mShaderResources[i].get();
        if (shader)
        {
            mShaderMacroMapping[i].Resize(mMacros.Size());
            rm->WaitForResource(shader);

            for (size_t j = 0; j < mMacros.Size(); ++j)
            {
                mShaderMacroMapping[i][j] = shader->GetMacroByName(mMacroNames[j].Str());
            }
        }
    }

    if (mMacros.Size() > 0)
    {
        std::unique_ptr<int[]> macroValues(new int[mMacros.Size()]);

        size_t totalCombinations = 1;
        for (size_t i = 0; i < mMacros.Size(); ++i)
        {
            const MultishaderMacro& macro = mMacros[i];
            totalCombinations *= (macro.maxValue - macro.minValue + 1);
            macroValues[i] = macro.minValue;
        }

        // generate and load all shader sets combinations
        for (size_t i = 0; i < totalCombinations; ++i)
        {
            LoadShaderSet(macroValues.get());

            macroValues[0]++;
            for (size_t j = 0; j < mMacros.Size() - 1; ++j)
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
        LoadShaderSet(nullptr);
    }
}

void MultiPipelineState::LoadShaderSet(int* macroValues)
{
    ShaderSet shaderSet;
    for (size_t i = 0; i < NFE_GRAPHICS_SHADER_TYPES_NUM; ++i)
    {
        if (!mShaderResources[i])
            continue;

        int shaderMacros[16] = { -1 };
        for (size_t j = 0; j < mMacros.Size(); ++j)
        {
            int dest = mShaderMacroMapping[i][j];
            if (dest >= 0)
                shaderMacros[dest] = macroValues[j];
        }

        const ShaderPtr& shader = mShaderResources[i]->GetShader(shaderMacros);
        ShaderType type = static_cast<ShaderType>(i);

        switch (type)
        {
        case ShaderType::Vertex:
            shaderSet.shaders[0] = shader;
            break;
        case ShaderType::Hull:
            shaderSet.shaders[1] = shader;
            break;
        case ShaderType::Domain:
            shaderSet.shaders[2] = shader;
            break;
        case ShaderType::Geometry:
            shaderSet.shaders[3] = shader;
            break;
        case ShaderType::Pixel:
            shaderSet.shaders[4] = shader;
            break;
        }
    }

    mShaderSets.PushBack(shaderSet);
}

bool MultiPipelineState::Build(const Renderer::PipelineStateDesc& desc)
{
    for (size_t i = 0; i < mShaderSets.Size(); ++i)
    {
        PipelineStateDesc psoDesc = desc;
        psoDesc.vertexShader = mShaderSets[i].shaders[0];
        psoDesc.hullShader = mShaderSets[i].shaders[1];
        psoDesc.domainShader = mShaderSets[i].shaders[2];
        psoDesc.geometryShader = mShaderSets[i].shaders[3];
        psoDesc.pixelShader = mShaderSets[i].shaders[4];

        HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
        PipelineStatePtr pso(renderer->GetDevice()->CreatePipelineState(psoDesc));

        if (!pso)
            return false;

        mSubPipelineStates.PushBack(std::move(pso));
    }

    return true;
}

uint32 MultiPipelineState::GetMacrosNumber() const
{
    return mMacros.Size();
}

int MultiPipelineState::GetMacroByName(const char* name) const
{
    for (size_t i = 0; i < mMacros.Size(); ++i)
    {
        if (mMacroNames[i] == name)
            return static_cast<int>(i);
    }

    return -1;
}

const PipelineStatePtr& MultiPipelineState::GetPipelineState(int* macroValues) const
{
    int subshaderId = 0;
    int multiplier = 1;

    // calculate subshader index based on macro values
    for (size_t i = 0; i < mMacros.Size(); ++i)
    {
        const MultishaderMacro& macro = mMacros[i];
        int val = macroValues != nullptr ? macroValues[i] : macro.defaultValue;

        // check if macro value is in range
        if (val < macro.minValue || val > macro.maxValue)
            val = macro.defaultValue;

        val -= macro.minValue;
        subshaderId += val * multiplier;
        multiplier *= (macro.maxValue - macro.minValue + 1);
    }

    return mSubPipelineStates[subshaderId];
}

const ShaderPtr& MultiPipelineState::GetShader(Renderer::ShaderType type, int* values) const
{
    int typeId = static_cast<int>(type);
    int macroValues[16] = { 0 };

    for (size_t i = 0; i < mMacros.Size(); ++i)
    {
        int mapping = mShaderMacroMapping[typeId][i];
        if (mapping >= 0)
            macroValues[mapping] = values != nullptr ? values[i] : mMacros[i].defaultValue;
    }

    return mShaderResources[typeId]->GetShader(macroValues);
}

int MultiPipelineState::GetResourceSlotByName(const char* slotName)
{
    int slot = -1;

    // resource slot ID must be the same for all sub pipeline states
    for (const auto& shaderSet : mShaderSets)
    {
        for (int i = 0; i < 5; ++i)
        {
            if (!shaderSet.shaders[i])
                continue;

            int currSlot = shaderSet.shaders[i]->GetResourceSlotByName(slotName);

            // name not present in this shader
            if (currSlot < 0)
                continue;

            if (slot < 0)
                slot = currSlot;
            if (currSlot != slot)
            {
                NFE_LOG_ERROR("Resource slot ID for slot name '%s' is mismatched in multi pipeline states '%s'", slotName, mName.Str());
                continue;
            }
        }
    }

    if (slot < 0)
    {
        NFE_LOG_ERROR("Resource slot '%s' not found in multi pipeline states '%s'", slotName, mName.Str());
    }

    return slot;
}

} // namespace Renderer
} // namespace NFE
