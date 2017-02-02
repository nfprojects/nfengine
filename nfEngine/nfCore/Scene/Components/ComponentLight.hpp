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
#include "../Systems/RenderProxies.hpp"

#include "nfCommon/Aligned.hpp"
#include "nfCommon/Math/Vector.hpp"


namespace NFE {
namespace Scene {

// maximum shadow map resolution
#define NFE_LIGHT_MAX_SHADOW_RES 4096


NFE_ALIGN16
class CORE_API LightComponent
    : public IComponent
    , public Common::Aligned<16>
{
private:
    Math::Vector mColor;

    // TODO resource hash
    Resource::Texture* mLightMap;

    RenderProxyID mRenderingProxy;

    Resource::LightType mLightType;

    uint16 mShadowMapResolution;

    union
    {
        Resource::OmniLightDesc mOmniLight;
        Resource::SpotLightDesc mSpotLight;
        Resource::DirLightDesc mDirLight;
    };

    void Release();
    void OnLightMapTextureLoaded();
    void CreateRenderingProxy(bool update);

public:
    LightComponent();
    ~LightComponent();

    // IComponent interface implementation
    virtual Math::Box GetBoundingBox() const override;
    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;

    void SetColor(const Math::Float3& color);
    void SetOmniLight(const Resource::OmniLightDesc& desc);
    void SetSpotLight(const Resource::SpotLightDesc& desc);
    void SetDirLight(const Resource::DirLightDesc& desc);

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
    bool SetShadowMap(uint16 resolution);
};

} // namespace Scene
} // namespace NFE
