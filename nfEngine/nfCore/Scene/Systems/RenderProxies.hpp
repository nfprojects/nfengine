/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system proxies.
 */

#pragma once

#include "../../Core.hpp"

#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Math/Matrix.hpp"


namespace NFE {
namespace Scene {

/**
 * Mesh Proxy Descriptor
 */
struct NFE_ALIGN16 MeshProxyDesc
{
    Math::Matrix transform;
    Math::Vector velocity;
    Math::Vector angularVelocity;

    // TODO: place IB and VB here instead?
    Resource::Mesh* mesh;

    NFE_INLINE MeshProxyDesc()
        : mesh(nullptr)
    { }
};


//////////////////////////////////////////////////////////////////////////

enum class LightProxyType : uint8
{
    Omni,
    Spot,
    // TODO more types
};

struct LightProxyOmni
{
    float radius;
};

struct LightProxySpot
{
    float nearDistance;
    float farDistance;

    Resource::Texture* projectionMap;

    NFE_INLINE LightProxySpot()
        : nearDistance(0.05f)
        , farDistance(100.0f)
        , projectionMap(nullptr)
    { }
};

/**
 * Light Proxy Descriptor
 */
struct NFE_ALIGN16 LightProxyDesc
{
    // HDR color value in linear space
    Math::Vector color;

    // global transformation matrix
    Math::Matrix transform;

    LightProxyType type;

    uint16 shadowMapSize;

    union
    {
        LightProxyOmni omni;
        LightProxySpot spot;
    };
};

using RenderProxyID = uint32;

static const RenderProxyID INVALID_RENDER_PROXY;

} // namespace Scene
} // namespace NFE
