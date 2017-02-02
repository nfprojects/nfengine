/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Body component declaration.
 */

#pragma once

#include "Component.hpp"
#include "../Systems/PhysicsSystem.hpp"
#include "../../Resources/CollisionShape.hpp"

#include "nfCommon/Memory/Aligned.hpp"


namespace NFE {
namespace Scene {

/**
 * Component representing physical body.
 */
class CORE_API NFE_ALIGN16 BodyComponent final
    : public IComponent
    , public Common::Aligned<16>
{
public:
    BodyComponent();
    ~BodyComponent();

    // IComponent interface implementation
    virtual Math::Box GetBoundingBox() const override;
    virtual void OnAttach() override;
    virtual void OnDetach() override;

    /**
     * Assigning a collision shape.
     */
    void SetCollisionShape(Resource::CollisionShape* shape);

    /**
     * Get cached velocity (in m/s).
     * @note    This value may not be up to date (it comes from previous frame).
     */
    const Math::Vector& GetVelocity() const { return mVelocity; }

    /**
     * Get cached angular velocity (in rad/s).
     * @note    This value may not be up to date (it comes from previous frame).
     */
    const Math::Vector& GetAngularVelocity() const { return mAngularVelocity; }

    /**
     * Change velocity (in m/s).
     */
    void SetVelocity(const Math::Vector& newVelocity);

    /**
     * Change angular velocity (in rad/s).
     */
    void SetAngularVelocity(const Math::Vector& newAngularVelocity);

    /**
     * Set body mass (in kg).
     * @param mass New mass value. When less or equal zero, the body is static (as if it has infinite mass).
     */
    void SetMass(float mass);

    /**
     * Get body mass (in kg).
     */
    float GetMass() const;

private:
    Math::Vector mVelocity;
    Math::Vector mAngularVelocity;

    Resource::CollisionShape* mCollisionShape;
    float mMass;

    PhysicsProxyID mPhysicsProxy;

    // get physics system managing this component
    IPhysicsSystem* GetPhysicsSystem() const;

    void CreatePhysicsProxy();
    void UpdatePhysicsProxy();
    void DeletePhysicsProxy();

    void OnBodyUpdateTransform(const Math::Matrix& newTransform);
    void OnBodyUpdateVelocities(const Math::Vector& newVelocity, const Math::Vector& newAngularVelocity);
};

} // namespace Scene
} // namespace NFE
