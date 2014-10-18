/**
    NFEngine project

    \file   Material.cpp
    \brief  Material resource declaration.
*/

#include "stdafx.hpp"
#include "Material.hpp"
#include "ResourcesManager.hpp"
#include "Engine.hpp"
#include "Globals.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Resource {

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

bool Material::OnLoad()
{
    LOG_INFO("Loading material '%s'...", mName);

    //get relative path
    std::string path = g_DataPath + "Materials/" + mName + ".cfg";

    config_t cfg;
    config_setting_t* pLayersNode, *pNode;
    config_init(&cfg);

    if (!config_read_file(&cfg, path.c_str()))
    {
        LOG_ERROR("Failed to load material '%s': %s (line %d)", mName, config_error_text(&cfg),
                  config_error_line(&cfg));
        config_destroy(&cfg);
        return false;
    }


    pLayersNode = config_lookup(&cfg, "Layers");

    if (pLayersNode == 0)
    {
        mLayers = new MaterialLayer [1];
        mLayersCount = 1;
        goto MaterialLoadedLabel;
    }

    mLayersCount = config_setting_length(pLayersNode);

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

        //extract layers array element
        config_setting_t* pLayer = config_setting_get_elem(pLayersNode, i);
        if (pLayer == 0) continue;

        const char* pName;
        if (config_setting_lookup_string(pLayer, "DiffuseTexture", &pName))
        {
            layer.diffuseTexture = ENGINE_GET_TEXTURE(pName);
            layer.diffuseTexture->AddRef();
        }

        if (config_setting_lookup_string(pLayer, "NormalTexture", &pName))
        {
            layer.normalTexture = ENGINE_GET_TEXTURE(pName);
            layer.normalTexture->AddRef();
        }

        if (config_setting_lookup_string(pLayer, "SpecularTexture", &pName))
        {
            layer.specularTexture = ENGINE_GET_TEXTURE(pName);
            layer.specularTexture->AddRef();
        }

        double specPower = 20.0f;
        config_setting_lookup_float(pLayer, "SpecularPower", &specPower);
        layer.specularColor.w = (float)specPower;

        double specFactor = 1.0f;
        config_setting_lookup_float(pLayer, "SpecularFactor", &specFactor);
        layer.specularColor.x = (float)specFactor;

        //look up for diffuse color
        layer.diffuseColor = Float4(1.0f, 1.0f, 1.0f, 1.0f);
        pNode = config_setting_get_member(pLayer, "DiffuseColor");
        if (pNode)
            if (config_setting_length(pNode) == 3)
            {
                layer.diffuseColor.x = (float)config_setting_get_float_elem(pNode, 0);
                layer.diffuseColor.y = (float)config_setting_get_float_elem(pNode, 1);
                layer.diffuseColor.z = (float)config_setting_get_float_elem(pNode, 2);
            }

        //look up for emission color
        layer.emissionColor = Float4();
        pNode = config_setting_get_member(pLayer, "EmissionColor");
        if (pNode)
            if (config_setting_length(pNode) == 3)
            {
                layer.emissionColor.x = (float)config_setting_get_float_elem(pNode, 0);
                layer.emissionColor.y = (float)config_setting_get_float_elem(pNode, 1);
                layer.emissionColor.z = (float)config_setting_get_float_elem(pNode, 2);
            }


        //config_setting_t *pLayer = config_sett
        //config_setting_lookup_float(
    }


    /*
    mwTokenizer tokenizer;
    if (tokenizer.LoadFromFile(path) == 0)
    {
        LOG_ERROR("Failed to load '%s'.", mName);
        return false;
    }

    //
    mLayers = new MaterialLayer [1];
    mLayersCount = 1;


    int errorOccurred = 0;
    mwString<char> token;
    while (tokenizer.GetToken(&token))
    {

        if (token == "DiffuseTexture")
        {
            if (tokenizer.Expect("="))
            {
                if (tokenizer.GetToken(&token))
                {
                    mLayers[0].diffuseTexture = (Texture*)g_pResManager->GetResource(token.c_str(), RES_TEXTURE);
                    mLayers[0].diffuseTexture->AddRef();
                }
                else
                    errorOccurred = 1;
            }
            else
                errorOccurred = 1;
        }
        else if (token == "NormalTexture")
        {

            if (tokenizer.Expect("="))
            {
                if (tokenizer.GetToken(&token))
                {
                    mLayers[0].normalTexture = (Texture*)g_pResManager->GetResource(token.c_str(), RES_TEXTURE);
                    mLayers[0].normalTexture->AddRef();
                }
                else
                    errorOccurred = 1;
            }
            else
                errorOccurred = 1;
        }
        else if (token == "SpecularTexture")
        {

            if (tokenizer.Expect("="))
            {
                if (tokenizer.GetToken(&token))
                {
                    mLayers[0].specularTexture = (Texture*)g_pResManager->GetResource(token.c_str(), RES_TEXTURE);
                    mLayers[0].specularTexture->AddRef();
                }
                else
                    errorOccurred = 1;
            }
            else
                errorOccurred = 1;
        }
        else
            errorOccurred = 1;
    }
    */


MaterialLoadedLabel:

    config_destroy(&cfg);
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
