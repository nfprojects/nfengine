/**
 * @file   Material.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Material resource definitions.
 */

#include "PCH.hpp"
#include "Material.hpp"
#include "ResourcesManager.hpp"
#include "Engine.hpp"
#include "Globals.hpp"
#include "../nfCommon/Logger.hpp"
#include "../rapidjson/include/rapidjson/document.h"
#include "../rapidjson/include/rapidjson/filestream.h"

namespace NFE {
namespace Resource {

namespace {

const char* ATTR_LAYERS = "Layers";
const char* ATTR_DIFFUSE_TEXTURE = "DiffuseTexture";
const char* ATTR_NORMAL_TEXTURE = "NormalTexture";
const char* ATTR_SPECULAR_TEXTURE = "SpecularTexture";
const char* ATTR_DIFFUSE_COLOR = "DiffuseColor";
const char* ATTR_EMISSION_COLOR = "EmissionColor";
const char* ATTR_SPECULAR_POWER = "SpecularPower";
const char* ATTR_SPECULAR_FACTOR = "SpecularFactor";

} // namespace

using namespace Math;

MaterialLayer::MaterialLayer()
{
    weight = 1.0f;

    diffuseTexture = 0;
    normalTexture = 0;
    specularTexture = 0;
    textureScale = Float2(1.0f, 1.0f);

    diffuseColor = Float4(1.0f, 1.0f, 1.0f, 1.0f);
    specularColor = Float4(1.0f, 1.0f, 1.0f, 20.0f);
    emissionColor = Float4(0, 0, 0, 0);
}


Material::Material()
{
    mLayers = nullptr;
    mLayersCount = 0;
}

Material::~Material()
{
    if (mLayers)
    {
        delete[] mLayers;
        mLayers = 0;
    }
}

void NodeToColor(const rapidjson::Value& val, Float4& ret)
{
    using namespace rapidjson;

    if (!val.IsArray())
        return;

    SizeType num = std::min<SizeType>(4, val.Size());
    for (SizeType i = 0; i < num; ++i)
    {
        if (!val[i].IsDouble())
            continue;
        (&ret.x)[i] = static_cast<float>(val[i].GetDouble());
    }
}

bool Material::OnLoad()
{
    LOG_INFO("Loading material '%s'...", mName);

    //get relative path
    std::string path = g_DataPath + "Materials/" + mName + ".json";

    // TODO: support for file in memory
    FILE* pFile = fopen(path.c_str(), "r");
    if (pFile == NULL)
    {
        LOG_ERROR("Failed to open file: %s", path);
        return false;
    }

    rapidjson::Document document;
    rapidjson::FileStream fileStream(pFile);
    document.ParseStream<0>(fileStream);
    fclose(pFile);
    if (!document.IsObject())
    {
        LOG_ERROR("Failed to load material '%s': %s", mName, document.GetParseError());
        return false;
    }

    if (!document.HasMember(ATTR_LAYERS) || !document[ATTR_LAYERS].IsArray())
    {
        // TODO: improve messages after logger is refactored
        LOG_ERROR("Failed to load material '%s'", mName);
        return false;
    }

    const rapidjson::Value& layersNode = document[ATTR_LAYERS];
    mLayersCount = layersNode.Size();

    if (mLayersCount < 1)
    {
        mLayers = new MaterialLayer [1];
        mLayersCount = 1;
        goto MaterialLoadedLabel;
    }

    //max 4 layers (TEMPORARY)
    if (mLayersCount > 4)
    {
        mLayersCount = 4;
    }

    mLayers = new MaterialLayer [mLayersCount];


    for (uint32 i = 0; i < mLayersCount; i++)
    {
        MaterialLayer& layer = mLayers[i];
        const auto& node = layersNode[i];

        if (node.HasMember(ATTR_DIFFUSE_TEXTURE))
        {
            if (node[ATTR_DIFFUSE_TEXTURE].IsString())
            {
                layer.diffuseTexture = ENGINE_GET_TEXTURE(node[ATTR_DIFFUSE_TEXTURE].GetString());
                layer.diffuseTexture->AddRef();
            }
            else
            {
                LOG_WARNING("%s attribute must be a string", ATTR_DIFFUSE_TEXTURE);
            }
        }

        if (node.HasMember(ATTR_NORMAL_TEXTURE))
        {
            if (node[ATTR_NORMAL_TEXTURE].IsString())
            {
                layer.normalTexture = ENGINE_GET_TEXTURE(node[ATTR_NORMAL_TEXTURE].GetString());
                layer.normalTexture->AddRef();
            }
            else
            {
                LOG_WARNING("%s attribute must be a string", ATTR_NORMAL_TEXTURE);
            }
        }

        if (node.HasMember(ATTR_SPECULAR_TEXTURE))
        {
            if (node[ATTR_SPECULAR_TEXTURE].IsString())
            {
                layer.specularTexture = ENGINE_GET_TEXTURE(node[ATTR_SPECULAR_TEXTURE].GetString());
                layer.specularTexture->AddRef();
            }
            else
            {
                LOG_WARNING("%s attribute must be a string", ATTR_SPECULAR_TEXTURE);
            }
        }

        double specPower = 20.0f;
        if (node.HasMember(ATTR_SPECULAR_POWER) && node[ATTR_SPECULAR_POWER].IsDouble())
            specPower = node[ATTR_SPECULAR_POWER].GetDouble();
        layer.specularColor.w = (float)specPower;

        double specFactor = 1.0f;
        if (node.HasMember(ATTR_SPECULAR_FACTOR) && node[ATTR_SPECULAR_FACTOR].IsDouble())
            specPower = node[ATTR_SPECULAR_FACTOR].GetDouble();
        layer.specularColor.x = (float)specFactor;

        //look up for diffuse color
        layer.diffuseColor = Float4(1.0f, 1.0f, 1.0f, 1.0f);
        if (node.HasMember(ATTR_DIFFUSE_COLOR))
            NodeToColor(node[ATTR_DIFFUSE_COLOR], layer.diffuseColor);

        //look up for emission color
        layer.emissionColor = Float4();
        if (node.HasMember(ATTR_EMISSION_COLOR))
            NodeToColor(node[ATTR_EMISSION_COLOR], layer.diffuseColor);
    }

MaterialLoadedLabel:
    LOG_SUCCESS("Material '%s' loaded successfully.", mName);
    return true;
}

void Material::OnUnload()
{
    LOG_INFO("Unloading material '%s'...", mName);

    if (mLayers)
    {
        //deleta all resource references
        for (uint32 i = 0; i < mLayersCount; i++)
        {
            if (mLayers[i].diffuseTexture != nullptr)
                mLayers[i].diffuseTexture->DelRef();

            if (mLayers[i].normalTexture != nullptr)
                mLayers[i].normalTexture->DelRef();

            if (mLayers[i].specularTexture != nullptr)
                mLayers[i].specularTexture->DelRef();
        }


        delete[] mLayers;
        mLayers = 0;
    }

    mLayersCount = 0;
}


using namespace Render;

/*
    This function is only temporary (to adapt renderer to the rest of code).
    It must be redesigned.
*/
const RendererMaterial* Material::GetRendererData()
{

    if (mRendererData.layersNum != mLayersCount)
    {
        if (mRendererData.layers != nullptr)
        {
            delete[] mRendererData.layers;
        }

        mRendererData.layers = new RendererMaterialLayer [mLayersCount];
        mRendererData.layersNum = mLayersCount;
    }

    // TODO: optimize
    for (uint32 i = 0; i < mLayersCount; i++)
    {
        mRendererData.layers[i].diffuseColor = mLayers[i].diffuseColor;
        mRendererData.layers[i].emissionColor = mLayers[i].emissionColor;
        mRendererData.layers[i].specularColor = mLayers[i].specularColor;

        if (mLayers[i].diffuseTexture != nullptr)
            mRendererData.layers[i].diffuseTex = mLayers[i].diffuseTexture->GetRendererTexture();
        else
            mRendererData.layers[i].diffuseTex = nullptr;

        if (mLayers[i].normalTexture != nullptr)
            mRendererData.layers[i].normalTex = mLayers[i].normalTexture->GetRendererTexture();
        else
            mRendererData.layers[i].normalTex = nullptr;

        if (mLayers[i].specularTexture != nullptr)
            mRendererData.layers[i].specularTex = mLayers[i].specularTexture->GetRendererTexture();
        else
            mRendererData.layers[i].specularTex = nullptr;
    }

    return &mRendererData;
}

} // namespace Resource
} // namespace NFE
