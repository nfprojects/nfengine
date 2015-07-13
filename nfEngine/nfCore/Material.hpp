/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Material resource declaration.
 */

#pragma once

#include "Core.hpp"
#include "Resource.hpp"
#include "Texture.hpp"
#include "Renderer/RendererResources.hpp"

namespace NFE {
namespace Resource {

/**
 * Structure describing material layer properties
 */
struct CORE_API MaterialLayer
{
    float weight; //how much the layer influences final color
    Math::Float2 textureScale;

    Texture* diffuseTexture;
    Texture* normalTexture;
    Texture* specularTexture;

    Math::Float4 diffuseColor;
    Math::Float4 specularColor; // x - factor, w - power
    Math::Float4 emissionColor;

    /*
    TODO:
        - emission texture
        - transparency
        - cubemap reflection (static & dynamic)
    */

    MaterialLayer();
};


/**
 * Material class - list of material layers
 */
class CORE_API Material : public ResourceBase
{
    friend class Renderer::GBufferRenderer;
    friend class Renderer::DebugRenderer;
    friend class Renderer::ShadowRenderer;

    // TODO: use std::vector
    MaterialLayer* mLayers;
    uint32 mLayersCount;

    Renderer::RendererMaterial mRendererData;

public:
    Material();
    ~Material();

    bool OnLoad();
    void OnUnload();

    /**
     * Get renderer's material
     */
    const Renderer::RendererMaterial* GetRendererData();
};

} // namespace Resource
} // namespace NFE
