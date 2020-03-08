/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of light component class.
 */

#pragma once

#include "Component.hpp"

#include "../../../Common/Memory/Aligned.hpp"
#include "../../../Common/Math/Vector4.hpp"


namespace NFE {
namespace Scene {

// maximum shadow map resolution
#define NFE_LIGHT_MAX_SHADOW_RES 4096

/**
 * Supported light types
 */
enum class LightType : uint8
{
    Unknown = 0,
    Omni,
    Spot,
    Dir,
    // TODO: line light, area light, etc/
};

/**
 * Omni directional light descriptor.
 */
struct OmniLightDesc
{
    float radius;            //< light sphere radius
    float shadowFadeStart;   //< start fading the shadow at this distance
    float maxShadowDistance; //< don't draw shadow if camera is further than this distance

    OmniLightDesc()
        : radius(10.0f)
        , shadowFadeStart(50.0f)
        , maxShadowDistance(75.0f)
    { }
};

/**
 * Spot light descriptor.
 */
struct SpotLightDesc
{
    float nearDist;          //< near distance of frustum's clipping plane
    float farDist;           //< far distance of frustum's clipping plane
    float cutoff;            //< cutoff angle (in radians)
    float maxShadowDistance; //< don't draw shadow if camera is further than this distance

    SpotLightDesc()
        : nearDist(0.1f)
        , farDist(50.0f)
        , cutoff(Math::DegToRad(50.0f))
        , maxShadowDistance(75.0f)
    { }
};

/**
 * Directional light descriptor.
 */
struct DirLightDesc
{
    float farDist;   //< how far cascaded shadow maps will be visible
    float lightDist; //< max distance from camera frustum to shadow caster
    int32 splits;    //< cascaded shadow maps splits number

    DirLightDesc()
        : farDist(100.0f)
        , lightDist(200.0f)
        , splits(4)
    { }
};

/**
 * A component spawning light proxy on a render scene.
 */
class NFE_ALIGN(16) CORE_API LightComponent
    : public IComponent
    , public Common::Aligned<16>
{
    NFE_DECLARE_POLYMORPHIC_CLASS(LightComponent)

private:
    Math::Vector4 mColor;

    LightType mLightType;

    uint16 mShadowMapResolution;

    union
    {
        OmniLightDesc mOmniLight;
        SpotLightDesc mSpotLight;
        DirLightDesc mDirLight;
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
    void SetOmniLight(const OmniLightDesc& desc);
    void SetSpotLight(const SpotLightDesc& desc);
    void SetDirLight(const DirLightDesc& desc);
};

} // namespace Scene
} // namespace NFE
