/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Body component declaration.
 */

#pragma once

#include "../../Core.hpp"
#include "Component.hpp"
#include "../../Resources/CollisionShape.hpp"
#include "nfCommon/Aligned.hpp"


namespace NFE {
namespace Scene {

/**
 * Component representing physical body.
 */
NFE_ALIGN16
class CORE_API BodyComponent final
    : public IComponent
    , public Common::Aligned<16>
{
private:
    Math::Vector mVelocity;
    Math::Vector mAngularVelocity;

    Resource::CollisionShape* mCollisionShape;
    float mMass;

public:
    BodyComponent();
    ~BodyComponent();

    // IComponent interface implementation
    virtual Math::Box GetBoundingBox() const override;
    virtual void OnAttach(Entity* entity) override;
    virtual void OnDetach() override;

    /**
     * Assigning a collision shape.
     */
    void SetCollisionShape(Resource::CollisionShape* shape);

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
