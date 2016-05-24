/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Material resource definitions.
 */

#include "PCH.hpp"
#include "Material.hpp"
#include "ResourcesManager.hpp"
#include "Engine.hpp"
#include "Globals.hpp"
#include "Renderer/GeometryRenderer.hpp"

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
    numTextures = 0;
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
}

Material::~Material()
{
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

    std::recursive_mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::unique_lock<std::recursive_mutex> lock(renderingMutex);

    ResManager* rm = Engine::GetInstance()->GetResManager();

    for (uint32 i = 0; i < layersNode.Size(); i++)
    {
        MaterialLayer layer;
        const auto& node = layersNode[i];

        if (node.HasMember(ATTR_DIFFUSE_TEXTURE))
        {
            if (node[ATTR_DIFFUSE_TEXTURE].IsString())
            {
                layer.diffuseTexture = static_cast<Texture*>(
                    rm->GetResource(node[ATTR_DIFFUSE_TEXTURE].GetString(), ResourceType::Texture));
                layer.diffuseTexture->AddRef();
                layer.numTextures++;
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
                layer.normalTexture = static_cast<Texture*>(
                    rm->GetResource(node[ATTR_NORMAL_TEXTURE].GetString(),ResourceType::Texture));
                layer.normalTexture->AddRef();
                layer.numTextures++;
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
                layer.specularTexture = static_cast<Texture*>(
                    rm->GetResource(node[ATTR_SPECULAR_TEXTURE].GetString(), ResourceType::Texture));
                layer.specularTexture->AddRef();
                layer.numTextures++;
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

        mLayers.push_back(layer);

        // register callbacks for each texture
        if (layer.diffuseTexture)
            layer.diffuseTexture->AddPostLoadCallback(std::bind(&Material::OnTextureLoaded, this));
        if (layer.normalTexture)
            layer.normalTexture->AddPostLoadCallback(std::bind(&Material::OnTextureLoaded, this));
        if (layer.specularTexture)
            layer.specularTexture->AddPostLoadCallback(std::bind(&Material::OnTextureLoaded, this));
    }

    if (mLayers.size() > 0)
    {
        mRendererData.layers[0].diffuseColor = mLayers[0].diffuseColor;
        mRendererData.layers[0].emissionColor = mLayers[0].emissionColor;
        mRendererData.layers[0].specularColor = mLayers[0].specularColor;
    }

    LOG_SUCCESS("Material '%s' loaded successfully.", mName);
    return true;
}

void Material::OnUnload()
{
    LOG_INFO("Unloading material '%s'...", mName);

    std::recursive_mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::unique_lock<std::recursive_mutex> lock(renderingMutex);

    mRendererData.layers[0].bindingInstance.reset();
    mLayers.clear();
}

void Material::OnTextureLoaded()
{
    using namespace Renderer;
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    std::recursive_mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::lock_guard<std::recursive_mutex> lock(renderingMutex);

    // TODO multi-layered textures
    if (mLayers.size() > 0)
    {
        // return if it was not the last texture required
        if (++mTexturesLoaded < mLayers[0].numTextures)
            return;

        auto& bindingInstance = mRendererData.layers[0].bindingInstance;
        bindingInstance.reset(renderer->GetDevice()->CreateResourceBindingInstance(
            GeometryRenderer::Get()->GetMaterialTexturesBindingSet()));
        if (!bindingInstance)
        {
            LOG_ERROR("Failed to create material's binding instance");
            return;
        }

        ITexture* diffuseTexture = renderer->GetDefaultDiffuseTexture();
        if (mLayers[0].diffuseTexture && mLayers[0].diffuseTexture->GetRendererTexture())
            diffuseTexture = mLayers[0].diffuseTexture->GetRendererTexture();
        if (!bindingInstance->WriteTextureView(0, diffuseTexture))
        {
            LOG_ERROR("Failed to write material's diffuse texture to binding instance");
            return;
        }

        ITexture* normalTexture = renderer->GetDefaultNormalTexture();
        if (mLayers[0].normalTexture && mLayers[0].normalTexture->GetRendererTexture())
            normalTexture = mLayers[0].normalTexture->GetRendererTexture();
        if (!bindingInstance->WriteTextureView(1, normalTexture))
        {
            LOG_ERROR("Failed to write material's diffuse texture to binding instance");
            return;
        }

        ITexture* specularTexture = renderer->GetDefaultSpecularTexture();
        if (mLayers[0].specularTexture && mLayers[0].specularTexture->GetRendererTexture())
            specularTexture = mLayers[0].specularTexture->GetRendererTexture();
        if (!bindingInstance->WriteTextureView(2, specularTexture))
        {
            LOG_ERROR("Failed to write material's diffuse texture to binding instance");
            return;
        }

        LOG_SUCCESS("Binding instance for material '%s' created successfully", mName);
    }
}

} // namespace Resource
} // namespace NFE
