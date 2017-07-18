/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "EntityTemplateComponent.hpp"

#include "nfCommon/Math/Vector.hpp"

#include <string>


namespace NFE {
namespace Resource {

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
class OmniLightDesc
{
    NFE_DECLARE_CLASS(OmniLightDesc)

public:
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
class SpotLightDesc
{
    NFE_DECLARE_CLASS(SpotLightDesc)

public:
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
class DirLightDesc
{
    NFE_DECLARE_CLASS(DirLightDesc)

public:
    float farDist;      //< how far cascaded shadow maps will be visible
    float lightDist;    //< max distance from camera frustum to shadow caster
    int8 splits;        //< cascaded shadow maps splits number

    DirLightDesc()
        : farDist(100.0f)
        , lightDist(200.0f)
        , splits(4)
    { }
};

/**
 * Serializable EntityTemplate's component that represents a Light Component.
 */
class CORE_API EntityTemplateComponentLight : public IEntityTemplateComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(EntityTemplateComponentLight)

public:
    Math::Float3 color;
    OmniLightDesc omni;
    SpotLightDesc spot;
    DirLightDesc dir;
    uint16 shadowMapSize;
    LightType lightType;

    Common::String lightProjectionTextureName; // TODO resource ID

    EntityTemplateComponentLight();

    virtual Scene::ComponentPtr CreateComponent() const override;
};

} // namespace Resource
} // namespace NFE
