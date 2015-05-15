/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Material resource declaration.
 */

#pragma once

#include "Core.hpp"
#include "Resource.hpp"
#include "Texture.hpp"
#include "Renderer.hpp"

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
    friend class GBufferRendererD3D11;
    friend class DebugRendererD3D11;
    friend class ShadowRendererD3D11;

    // TODO: use std::vector
    MaterialLayer* mLayers;
    uint32 mLayersCount;

    Render::RendererMaterial mRendererData;

public:
    Material();
    ~Material();

    bool OnLoad();
    void OnUnload();

    /**
     * Get renderer's material
     */
    const Render::RendererMaterial* GetRendererData();
};

} // namespace Resource
} // namespace NFE
