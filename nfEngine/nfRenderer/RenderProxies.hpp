/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of renderer system proxies.
 */

#pragma once

#include "nfRenderer.hpp"

#include "nfCommon/Math/Vector4.hpp"
#include "nfCommon/Math/Matrix4.hpp"


namespace NFE {
namespace Renderer {


// TODO material proxy


/**
 * Mesh Proxy Descriptor
 */
struct NFE_ALIGN(16) MeshProxyDesc
{
    Math::Matrix4 transform;
    Math::Vector4 velocity;
    Math::Vector4 angularVelocity;

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
    Math::Vector4 color;

    // global transformation matrix
    Math::Matrix4 transform;

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
