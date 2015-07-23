/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of light component class.
 */

#pragma once

#include "../Core.hpp"
#include "Component.hpp"
#include "CameraComponent.hpp"
#include "../Texture.hpp"

namespace NFE {
namespace Scene {

// maximum shadow map resolution
#define X_LIGHT_MAX_SHADOW_RES 4096

/** Supported light types. */
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
    //cube light map
    float radius;
    float shadowFadeStart; // start fading the shadow at this distance
    float shadowFadeEnd;   // don't draw shadow if camera is further than this distance
};

struct SpotLightDesc
{
    //light map
    float nearDist;
    float farDist;
    float cutoff;
    float maxShadowDistance; // don't draw shadow if camera is further than this distance
};

struct DirLightDesc
{
    float farDist;   //how far cascaded shadow maps will affect
    float lightDist; //max distance from camera frustum to shadow caster
    int32 splits;    //shadow maps
};

/** Light entity descriptor used for serialization. */
#pragma pack(push, 1)
struct LightDesc
{
    char lightType;   //cast to LightType
    Math::Float3 color;

    union //select one depending on lightType value
    {
        OmniLightDesc omni;
        SpotLightDesc spot;
        DirLightDesc dir;
    };

    uint16 mShadowMapSize;
    char mLightMapName[RES_NAME_MAX_LENGTH];
};
#pragma pack(pop)

class CORE_API LightComponent : public Component
{
    friend class SceneManager;
    friend class RendererSystem;

    friend void DrawShadowMapCallback(void* pUserData, int Instance, int ThreadID);

    Math::Vector mColor;
    LightType mLightType;
    bool mUpdateShadowmap;
    bool mDrawShadow;

    OmniLightDesc mOmniLight;
    SpotLightDesc mSpotLight;
    DirLightDesc mDirLight;

    // list of cameras used during shadow maps rendering
    std::vector<CameraComponent*> mCameras;

    Math::Vector mCascadeRanges[8];
    Renderer::ShadowMap* mShadowMap;

    Resource::Texture* mLightMap;

    void Release();
    bool CanBeTiled();
    void CheckShadowVisibility(const Math::Vector& camPos);

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
     * @return Result::OK on success
     */
    Result SetShadowMap(uint32 resolution);
    bool HasShadowMap() const;

    /**
     * Test intersection with a frustum shape.
     */
    int IntersectFrustum(const Math::Frustum& frustum);

    void Update(CameraComponent* pCamera);
};

} // namespace Scene
} // namespace NFE
