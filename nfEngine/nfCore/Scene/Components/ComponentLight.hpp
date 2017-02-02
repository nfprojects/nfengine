/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of light component class.
 */

#pragma once

#include "../../Core.hpp"
#include "Component.hpp"
#include "../../Resources/GameObject/GameObjectComponentLight.hpp"
#include "../../Resources/Texture.hpp"
#include "nfCommon/Aligned.hpp"
#include "nfCommon/Math/Vector.hpp"


namespace NFE {
namespace Scene {

// maximum shadow map resolution
#define NFE_LIGHT_MAX_SHADOW_RES 4096


NFE_ALIGN16
class CORE_API LightComponent
    : public Component
    , public Common::Aligned<16>
{
private:
    Math::Vector mColor;

    Resource::LightType mLightType;

    union
    {
        Resource::OmniLightDesc mOmniLight;
        Resource::SpotLightDesc mSpotLight;
        Resource::DirLightDesc mDirLight;
    };

    // TODO resource hash
    Resource::Texture* mLightMap;

    // TODO move to rendering proxy
    std::unique_ptr<Renderer::ShadowMap> mShadowMap;
    std::unique_ptr<Renderer::IResourceBindingInstance> mLightMapBindingInstance;

    void Release();
    bool CanBeTiled();
    void OnLightMapTextureLoaded();

public:
    LightComponent();
    ~LightComponent();

    void SetColor(const Math::Float3& color);
    void SetOmniLight(const Resource::OmniLightDesc* desc);
    void SetSpotLight(const Resource::SpotLightDesc* desc);
    void SetDirLight(const Resource::DirLightDesc* desc);

    /**
     * Set light map texture.
     * TODO: use pointer to a texture instead of name
     */
    void SetLightMap(const char* name);

    /**
     * Set shadow map resolution.
     * @param resolution Shadow map dimensions in pixels.
     * @return "true" on success
     */
    bool SetShadowMap(uint32 resolution);

    /**
     * Check if the light has shadowmap enabled.
     */
    bool HasShadowMap() const;
};

} // namespace Scene
} // namespace NFE
