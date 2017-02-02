/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Body component definition.
*/

#include "PCH.hpp"
#include "ComponentBody.hpp"
#include "../Entity.hpp"
#include "../SceneManager.hpp"
#include "Resources/ResourcesManager.hpp"
#include "Resources/CollisionShape.hpp"

#include "nfCommon/Utils/InputStream.hpp"
#include "nfCommon/Utils/OutputStream.hpp"


namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;

BodyComponent::BodyComponent()
    : mCollisionShape(nullptr)
    , mMass(0.0f)
    , mPhysicsProxy(INVALID_PHYSICS_PROXY)
{
}

BodyComponent::~BodyComponent()
{
    DeletePhysicsProxy();

    // TODO use "normal" resource pointer
    if (mCollisionShape != nullptr)
    {
        mCollisionShape->DelRef();
        mCollisionShape = nullptr;
    }
}

Box BodyComponent::GetBoundingBox() const
{
    // TODO
    return Box();
}

void BodyComponent::OnAttach()
{

}

void BodyComponent::OnDetach()
{

}

void BodyComponent::SetCollisionShape(CollisionShape* shape)
{
    // TODO use resource handle

    if (mCollisionShape)
        mCollisionShape->DelRef();

    mCollisionShape = shape;
    if (mCollisionShape)
        mCollisionShape->AddRef();
}

Math::Vector BodyComponent::GetVelocity() const
{
    // TODO get velocity from physics engine
    return mVelocity;
}

Math::Vector BodyComponent::GetAngularVelocity() const
{
    // TODO get velocity from physics engine
    return mAngularVelocity;
}

void BodyComponent::SetVelocity(const Math::Vector& newVelocity)
{
    mVelocity = newVelocity;
    // TODO set velocity
}

void BodyComponent::SetAngularVelocity(const Math::Vector& newAngularVelocity)
{
    mAngularVelocity = newAngularVelocity;
    // TODO set velocity
}

float BodyComponent::GetMass() const
{
    // TODO get mass from physics engine
    return mMass;
}

void BodyComponent::SetMass(float mass)
{
    mMass = mass;
    // TODO set mass
}

void BodyComponent::CreatePhysicsProxy()
{
    // cleanup
    DeletePhysicsProxy();

    Entity* entity = GetEntity();
    if (!GetEntity())
        return;

    SceneManager* scene = entity->GetScene();
    NFE_ASSERT(scene, "Entity not attached to a scene");

    PhysicsBodyProxyInfo proxyInfo;
    proxyInfo.collisionShape = mCollisionShape;
    proxyInfo.mass = mMass;
    proxyInfo.transform = entity->GetGlobalTransform().ToMatrix();
    proxyInfo.inertia = Vector(1.0f, 1.0f, 1.0f); // TODO

    IPhysicsSystem* physicsSystem = scene->GetPhysicsSystem();
    NFE_ASSERT(physicsSystem, "Physics system is not present");
    mPhysicsProxy = physicsSystem->CreateBodyProxy(proxyInfo);
}

void BodyComponent::DeletePhysicsProxy()
{
    if (mPhysicsProxy == INVALID_PHYSICS_PROXY)
        return;

    Entity* entity = GetEntity();
    if (!GetEntity())
        return;

    SceneManager* scene = entity->GetScene();
    NFE_ASSERT(scene, "Entity not attached to a scene");

    IPhysicsSystem* physicsSystem = scene->GetPhysicsSystem();
    NFE_ASSERT(physicsSystem, "Physics system is not present");
    physicsSystem->DeleteBodyProxy(mPhysicsProxy);
    mPhysicsProxy = INVALID_PHYSICS_PROXY;
}

} // namespace Scene
} // namespace NFE
