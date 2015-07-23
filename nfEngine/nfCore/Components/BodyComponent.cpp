/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Body component definition.
*/

#include "../PCH.hpp"
#include "BodyComponent.hpp"
#include "../ResourcesManager.hpp"
#include "../CollisionShape.hpp"
#include "../nfCommon/InputStream.hpp"
#include "../nfCommon/OutputStream.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;

namespace {

/**
 * Convert Bullet's vector NFEngine's Vector
 */
Vector bt2Vector(const btVector3& v)
{
    return Vector((float*)v.m_floats);
}

/**
 * Convert NFEngine's Vector to Bullet's vector
 */
btVector3 Vector2bt(const Vector& v)
{
    return btVector3(v.f[0], v.f[1], v.f[2]);
}

} // namespace

BodyComponent::BodyComponent()
{
    mCollisionShape = nullptr;
    mMass = 0.0f;
    mFlags = 0;
}

BodyComponent::~BodyComponent()
{
    DisablePhysics();

    if (mCollisionShape)
    {
        mCollisionShape->DelRef();
        mCollisionShape = 0;
    }
}

void BodyComponent::EnablePhysics(CollisionShape* shape)
{
    if (mRigidBody)
        return;

    if (mCollisionShape)
        mCollisionShape->DelRef();

    mCollisionShape = shape;
    mCollisionShape->AddRef();

    mFlags |= BODY_COMPONENT_FLAG_INIT;
}

void BodyComponent::DisablePhysics()
{
    mFlags |= BODY_COMPONENT_FLAG_RELEASE;
}

Math::Vector BodyComponent::GetVelocity() const
{
    if (mRigidBody)
        return bt2Vector(mRigidBody->getLinearVelocity());

    return Vector();
}

Math::Vector BodyComponent::GetAngularVelocity() const
{
    if (mRigidBody)
        return bt2Vector(mRigidBody->getAngularVelocity());

    return Vector();
}

void BodyComponent::SetVelocity(const Math::Vector& newVelocity)
{
    if (mRigidBody)
        mRigidBody->setLinearVelocity(Vector2bt(newVelocity));
}

void BodyComponent::SetAngularVelocity(const Math::Vector& newAngularVelocity)
{
    if (mRigidBody)
        mRigidBody->setAngularVelocity(Vector2bt(newAngularVelocity));
}

void BodyComponent::SetMass(float mass)
{
    mMass = mass;

    if (mRigidBody && mCollisionShape)
    {
        btVector3 inertia = btVector3(mCollisionShape->mLocalInertia.f[0],
                                      mCollisionShape->mLocalInertia.f[1],
                                      mCollisionShape->mLocalInertia.f[2]);

        mRigidBody->setMassProps(mMass, inertia * mMass);
    }
}

float BodyComponent::GetMass()
{
    return mMass;
}

} // namespace Scene
} // namespace NFE
