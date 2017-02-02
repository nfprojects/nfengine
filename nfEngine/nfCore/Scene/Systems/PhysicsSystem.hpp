/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of physics system.
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"

#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Math/Matrix.hpp"


namespace NFE {
namespace Scene {

// TODO translation + orientation
using PhysicsBodyProxyTransformUpdateCallback = std::function<void(const Math::Matrix&)>;
using PhysicsBodyProxyVelocityUpdateCallback = std::function<void(const Math::Vector&, const Math::Vector&)>;
using PhysicsProxyID = uint32;

static const PhysicsProxyID INVALID_PHYSICS_PROXY = UINT32_MAX;


// TODO joins
// TODO physics materials
// TODO force fields
// TODO destruction
// TODO cloth
// TODO soft body
// TODO fluid

/**
 * Physics body proxy properties.
 */
struct NFE_ALIGN16 PhysicsBodyProxyInfo
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
    PhysicsBodyProxyTransformUpdateCallback transformUpdateCallback;

    // this callback will be called when body's velocity has changed
    PhysicsBodyProxyVelocityUpdateCallback velocityUpdateCallback;

    // Mass in kg. Zero means infinite mass
    float mass;

    // TODO material properties
    // TODO drag/friction parameters
    // TODO CCD

    PhysicsBodyProxyInfo()
        : collisionShape(nullptr)
        , mass(0.0f)
    { }
};


/**
 * Physics system interface.
 *
 * @note Extracting the interface from the actual implementation hides
 * underlying physics library (Bullet in this case), so there is no symbols pollution.
 * It could be also possible to implement other "physics backends" in the future.
 */
class NFE_ALIGN16 IPhysicsSystem : public ISystem
{
public:
    IPhysicsSystem(SceneManager* scene)
        : ISystem(scene)
    { }

    /**
     * Create a new physics body proxy.
     */
    virtual PhysicsProxyID CreateBodyProxy(const PhysicsBodyProxyInfo& info) = 0;

    /**
     * Update existing physics body proxy. All the properties will be overridden.
     * @return  False if properties are corrupted.
     * @note    Callback functions are ignored.
     */
    virtual bool UpdateBodyProxy(const PhysicsProxyID proxyID, const PhysicsBodyProxyInfo& info) = 0;

    /**
     * Delete existing physics body proxy. Proxy ID must be valid.
     */
    virtual void DeleteBodyProxy(const PhysicsProxyID proxyID) = 0;
};

} // namespace Scene
} // namespace NFE
