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


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::BodyComponent)
    NFE_CLASS_PARENT(NFE::Scene::IComponent)
NFE_END_DEFINE_CLASS()


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
    CreatePhysicsProxy();
}

void BodyComponent::OnDetach()
{
    DeletePhysicsProxy();
}

IPhysicsSystem* BodyComponent::GetPhysicsSystem() const
{
    Entity* entity = GetEntity();
    if (!entity)
        return nullptr;

    SceneManager* scene = entity->GetScene();
    NFE_ASSERT(scene, "Entity not attached to a scene");

    IPhysicsSystem* physicsSystem = scene->GetPhysicsSystem();
    NFE_ASSERT(physicsSystem, "Physics system is not present");

    return physicsSystem;
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

void BodyComponent::SetVelocity(const Math::Vector& newVelocity)
{
    mVelocity = newVelocity;

    UpdatePhysicsProxy();
}

void BodyComponent::SetAngularVelocity(const Math::Vector& newAngularVelocity)
{
    mAngularVelocity = newAngularVelocity;

    UpdatePhysicsProxy();
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

void BodyComponent::OnBodyUpdateTransform(const Math::Matrix& newTransform)
{
    const Math::Transform transform = Math::Transform::FromMatrix(newTransform);
    GetEntity()->SetGlobalTransform(transform);
}

void BodyComponent::OnBodyUpdateVelocities(const Math::Vector& newVelocity, const Math::Vector& newAngularVelocity)
{
    mVelocity = newVelocity;
    mAngularVelocity = newAngularVelocity;
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

    const auto updateCallback = [this](const Math::Matrix& newTransform) { OnBodyUpdateTransform(newTransform); };

    PhysicsBodyProxyInfo proxyInfo;
    proxyInfo.collisionShape = mCollisionShape;
    proxyInfo.mass = mMass;
    proxyInfo.transform = entity->GetGlobalTransform().ToMatrix();
    proxyInfo.inertia = Vector(1.0f, 1.0f, 1.0f); // TODO
    proxyInfo.velocity = mVelocity;
    proxyInfo.angularVelocity = mAngularVelocity;
    proxyInfo.transformUpdateCallback = [this](const Math::Matrix& newTransform)
    {
        OnBodyUpdateTransform(newTransform);
    };
    proxyInfo.velocityUpdateCallback = [this](const Math::Vector& newVelocity, const Math::Vector& newAngularVelocity)
    {
        OnBodyUpdateVelocities(newVelocity, newAngularVelocity);
    };

    IPhysicsSystem* physicsSystem = scene->GetPhysicsSystem();
    NFE_ASSERT(physicsSystem, "Physics system is not present");
    mPhysicsProxy = physicsSystem->CreateBodyProxy(proxyInfo);
}

void BodyComponent::UpdatePhysicsProxy()
{
    if (mPhysicsProxy == INVALID_PHYSICS_PROXY)
    {
        CreatePhysicsProxy();
        return;
    }

    Entity* entity = GetEntity();
    if (!GetEntity())
        return;

    PhysicsBodyProxyInfo proxyInfo;
    proxyInfo.collisionShape = mCollisionShape;
    proxyInfo.mass = mMass;
    proxyInfo.transform = entity->GetGlobalTransform().ToMatrix();
    proxyInfo.inertia = Vector(1.0f, 1.0f, 1.0f); // TODO
    proxyInfo.velocity = mVelocity;
    proxyInfo.angularVelocity = mAngularVelocity;

    IPhysicsSystem* physicsSystem = GetPhysicsSystem();
    physicsSystem->UpdateBodyProxy(mPhysicsProxy, proxyInfo);
}

void BodyComponent::DeletePhysicsProxy()
{
    if (mPhysicsProxy == INVALID_PHYSICS_PROXY)
        return;

    IPhysicsSystem* physicsSystem = GetPhysicsSystem();
    physicsSystem->DeleteBodyProxy(mPhysicsProxy);
    mPhysicsProxy = INVALID_PHYSICS_PROXY;
}

} // namespace Scene
} // namespace NFE
