/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Body component definition.
*/

#include "PCH.hpp"
#include "BodyComponent.hpp"
#include "Resources/ResourcesManager.hpp"
#include "Resources/CollisionShape.hpp"

#include "nfCommon/InputStream.hpp"
#include "nfCommon/OutputStream.hpp"

#include "btBulletDynamicsCommon.h"


namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;

namespace {

/**
 * Convert Bullet's vector to NFEngine's Vector
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

NFE_REGISTER_COMPONENT(BodyComponent);

BodyComponent::BodyComponent()
    : mCollisionShape(nullptr)
    , mMass(0.0f)
    , mFlags(0)
{
}

BodyComponent::~BodyComponent()
{
    if (mCollisionShape != nullptr)
    {
        mCollisionShape->DelRef();
        mCollisionShape = nullptr;
    }
}

void BodyComponent::SetShape(CollisionShape* shape)
{
    if (mRigidBody)
        return;

    if (mCollisionShape)
        mCollisionShape->DelRef();

    mCollisionShape = shape;
    if (mCollisionShape)
        mCollisionShape->AddRef();

    mFlags |= BODY_COMPONENT_FLAG_INIT;
}

void BodyComponent::Invalidate()
{
    mFlags |= BODY_COMPONENT_FLAG_RELEASE;
}

Math::Vector BodyComponent::GetVelocity() const
{
    if (mRigidBody)
        return bt2Vector(mRigidBody->getLinearVelocity());

    return mVelocity;
}

Math::Vector BodyComponent::GetAngularVelocity() const
{
    if (mRigidBody)
        return bt2Vector(mRigidBody->getAngularVelocity());

    return mAngularVelocity;
}

void BodyComponent::SetVelocity(const Math::Vector& newVelocity)
{
    mVelocity = newVelocity;
    if (mRigidBody)
    {
        mRigidBody->activate();
        mRigidBody->setLinearVelocity(Vector2bt(newVelocity));
    }
}

void BodyComponent::SetAngularVelocity(const Math::Vector& newAngularVelocity)
{
    mAngularVelocity = newAngularVelocity;
    if (mRigidBody)
    {
        mRigidBody->activate();
        mRigidBody->setAngularVelocity(Vector2bt(newAngularVelocity));
    }
}

void BodyComponent::SetMass(float mass)
{
    mMass = mass;
    if (mRigidBody && mCollisionShape)
    {
        btVector3 inertia = Vector2bt(mCollisionShape->mLocalInertia);
        mRigidBody->setMassProps(mMass, inertia * mMass);
    }
}

} // namespace Scene
} // namespace NFE
