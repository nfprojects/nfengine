/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Body component declaration.
 */

#pragma once

#include "../Core.hpp"
#include "Component.hpp"
#include "../Resources/CollisionShape.hpp"
#include "nfCommon/Aligned.hpp"

class btDefaultMotionState;
class btRigidBody;

namespace NFE {
namespace Scene {

/**
 * Component representing physical body.
 */
NFE_ALIGN16
class CORE_API BodyComponent
    : public Component
    , public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(BodyComponent)
    NFE_MAKE_NONMOVEABLE(BodyComponent)

private:
    Math::Vector mVelocity;
    Math::Vector mAngularVelocity;

    Resource::CollisionShape* mCollisionShape;
    int mFlags;
    float mMass;

public:
    BodyComponent();
    ~BodyComponent();

    void Invalidate();

    /**
     * Assigning a collision shape.
     */
    void SetCollisionShape(Resource::CollisionShape* shape);

    /**
     * Disable physics interaction
     */
    void DisablePhysics();

    /**
     * Get current linear velocity.
     */
    Math::Vector GetVelocity() const;

    /**
     * Get current angular velocity (in radians per second).
     */
    Math::Vector GetAngularVelocity() const;

    /**
     * Change linear velocity.
     */
    void SetVelocity(const Math::Vector& newVelocity);

    /**
     * Change angular velocity (in radians per second).
     */
    void SetAngularVelocity(const Math::Vector& newAngularVelocity);

    /**
     * Set body mass
     * @param mass New mass value. When less or equal zero, the body is static.
     */
    void SetMass(float mass);

    float GetMass() const;
};

} // namespace Scene
} // namespace NFE
