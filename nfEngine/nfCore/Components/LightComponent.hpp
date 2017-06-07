/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of light component class.
 */

#pragma once

#include "../Core.hpp"
#include "Component.hpp"
#include "../Resources/Texture.hpp"
#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Math/Vector.hpp"


namespace NFE {
namespace Scene {

// maximum shadow map resolution
#define NFE_LIGHT_MAX_SHADOW_RES 4096

/**
 * Supported light types
 */
enum class LightType
{
    Unknown = 0,
    Omni,
    Spot,
    Dir,
    // TODO: line light
};

struct OmniLightDesc
{
    float radius;            //< light sphere radius
    float shadowFadeStart;   //< start fading the shadow at this distance
    float maxShadowDistance; //< don't draw shadow if camera is further than this distance
};

struct SpotLightDesc
{
    float nearDist;          //< near distance of frustum's clipping plane
    float farDist;           //< far distance of frustum's clipping plane
    float cutoff;            //< cutoff angle (in radians)
    float maxShadowDistance; //< don't draw shadow if camera is further than this distance
};

struct DirLightDesc
{
    float farDist;   //< how far cascaded shadow maps will be visible
    float lightDist; //< max distance from camera frustum to shadow caster
    int32 splits;    //< cascaded shadow maps splits number
};

/**
 * Light entity descriptor used for serialization.
 */
#pragma pack(push, 1)
struct LightDesc
{
    char lightType;   //< cast to LightType
    Math::Float3 color;

    // select one depending on lightType value
    union
    {
        OmniLightDesc omni;
        SpotLightDesc spot;
        DirLightDesc dir;
    };

    uint16 mShadowMapSize;
    char mLightMapName[RES_NAME_MAX_LENGTH];
};
#pragma pack(pop)

NFE_ALIGN(16)
class CORE_API LightComponent
    : public ComponentBase<LightComponent>
    , public Common::Aligned<16>
{
    friend class SceneManager;
    friend class RendererSystem;

    Math::Vector mColor;

    union
    {
        OmniLightDesc mOmniLight;
        SpotLightDesc mSpotLight;
        DirLightDesc mDirLight;
    };

    std::unique_ptr<Renderer::ShadowMap> mShadowMap;
    Resource::Texture* mLightMap;
    Renderer::ResourceBindingInstancePtr mLightMapBindingInstance;

    LightType mLightType;


    void Release();
    bool CanBeTiled();
    void OnLightMapTextureLoaded();

public:
    LightComponent();
    ~LightComponent();

    void SetColor(const Math::Float3& color);
    void SetOmniLight(const OmniLightDesc* pDesc);
    void SetSpotLight(const SpotLightDesc* pDesc);
    void SetDirLight(const DirLightDesc* pDesc);

    /**
     * Set light map texture.
     * TODO: use pointer to a texture instead of name
     */
    void SetLightMap(const char* pName);

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
