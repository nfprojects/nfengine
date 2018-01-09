/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system proxies.
 */

#pragma once

#include "../Core.hpp"

#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Math/Matrix.hpp"


namespace NFE {
namespace Renderer {


// TODO material proxy


/**
 * Mesh Proxy Descriptor
 */
struct NFE_ALIGN(16) MeshProxyDesc
{
    Math::Matrix transform;
    Math::Vector velocity;
    Math::Vector angularVelocity;

    // TODO place IB and VB here instead
    Resource::Mesh* mesh;

    MeshProxyDesc()
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
    float cutoffAngle; // in radians

    Resource::Texture* projectionMap;

    LightProxySpot()
        : nearDistance(0.05f)
        , farDistance(100.0f)
        , projectionMap(nullptr)
    { }
};

/**
 * Light Proxy Descriptor
 */
struct NFE_ALIGN(16) LightProxyDesc
{
    // HDR color value in linear space
    Math::Vector color;

    // global transformation matrix
    Math::Matrix transform;

    LightProxyType type;

    uint16 shadowMapSize;

    LightProxyOmni omni;
    LightProxySpot spot;

    LightProxyDesc()
        : type(LightProxyType::Omni)
        , shadowMapSize(0)
    {
    }
};

using ProxyID = uint32;

static const ProxyID INVALID_RENDER_PROXY = UINT32_MAX;

} // namespace Renderer
} // namespace NFE