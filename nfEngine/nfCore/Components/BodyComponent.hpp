/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Body component declaration.
 */

#pragma once

#include "../Core.hpp"
#include "Component.hpp"
#include "../Resources/CollisionShape.hpp"
#include "nfCommon/Memory/Aligned.hpp"

class btDefaultMotionState;
class btRigidBody;

namespace NFE {
namespace Scene {

/**
 * Body entity descriptor used for serialization.
 */
#pragma pack(push, 1)
struct BodyComponentDesc
{
    float mass;
    char collisionShapeName[RES_NAME_MAX_LENGTH];
};
#pragma pack(pop)

#define BODY_COMPONENT_FLAG_INIT (1<<0)
#define BODY_COMPONENT_FLAG_RELEASE (1<<1)


/**
 * Component representing physical body.
 */
NFE_ALIGN16
class CORE_API BodyComponent
    : public ComponentBase<BodyComponent>
    , public Common::Aligned<16>
{
    NFE_MAKE_NONCOPYABLE(BodyComponent)
    NFE_MAKE_NONMOVEABLE(BodyComponent)

    friend class PhysicsSystem;
    friend class RendererSystem;

private:
    Math::Vector mVelocity;
    Math::Vector mAngularVelocity;

    // TODO: these shouldn't be allocated dynamically
    std::unique_ptr<btDefaultMotionState> mMotionState;
    std::unique_ptr<btRigidBody> mRigidBody;

    Resource::CollisionShape* mCollisionShape;
    int mFlags;
    float mMass;

public:
    BodyComponent();
    ~BodyComponent();

    void Invalidate();

    /**
     * Enable physics interaction via assigning a collision shape
     */
    void EnablePhysics(Resource::CollisionShape* shape);

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
