/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of physics system.
 */

#pragma once

#include "../Core.hpp"

#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Math/Matrix.hpp"

#include <functional>


namespace NFE {
namespace Physics {


// TODO translation + orientation
using BodyProxyTransformUpdateCallback = std::function<void(const Math::Matrix&)>;
using BodyProxyVelocityUpdateCallback = std::function<void(const Math::Vector&, const Math::Vector&)>;
using ProxyID = uint32;

static constexpr ProxyID InvalidPhysicsProxyID = std::numeric_limits<ProxyID>::max();


/**
 * Physics body proxy properties.
 */
struct NFE_ALIGN(16) BodyProxyInfo
{
    // body transformation matrix
    Math::Matrix transform;

    // moment of inertia vector
    Math::Vector inertia;

    Math::Vector velocity;
    Math::Vector angularVelocity;

    // collision shape resource, can be null
    Resource::CollisionShape* collisionShape;

    // this callback will be called when body was moved
    BodyProxyTransformUpdateCallback transformUpdateCallback;

    // this callback will be called when body's velocity has changed
    BodyProxyVelocityUpdateCallback velocityUpdateCallback;

    // Mass in kg. Zero means infinite mass
    float mass;

    // TODO material properties
    // TODO drag/friction parameters
    // TODO CCD

    BodyProxyInfo()
        : collisionShape(nullptr)
        , mass(0.0f)
    { }
};

} // namespace Physics
} // namespace NFE
