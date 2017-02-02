/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"
#include "GameObjectComponent.hpp"

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
struct OmniLightDesc
{
    float radius;            //< light sphere radius
    float shadowFadeStart;   //< start fading the shadow at this distance
    float maxShadowDistance; //< don't draw shadow if camera is further than this distance

    NFE_INLINE OmniLightDesc()
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

    NFE_INLINE SpotLightDesc()
        : nearDist(0.1f)
        , farDist(50.0f)
        , cutoff(50.0f * NFE_MATH_PI / 180.0f)
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

    NFE_INLINE DirLightDesc()
        : farDist(100.0f)
        , lightDist(200.0f)
        , splits(4)
    { }
};

/**
 * Serializable game object component that represents a Light.
 */
class CORE_API GameObjectComponentLight : public IGameObjectComponent
{
public:
    LightType lightType;   //< cast to LightType
    Math::Float3 color;

    // select one depending on lightType value
    union
    {
        OmniLightDesc omni;
        SpotLightDesc spot;
        DirLightDesc dir;
    };

    uint16 shadowMapSize;

    std::string lightProjectionTextureName; // TODO resource ID

    GameObjectComponentLight();

    virtual Scene::IComponent* CreateComponent() override;
};

} // namespace Resource
} // namespace NFE
