/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Body component definition.
*/

#include "PCH.hpp"
#include "ComponentBody.hpp"
#include "../Entity.hpp"
#include "../SceneManager.hpp"
#include "../Systems/PhysicsSystem.hpp"
#include "../../Physics/PhysicsScene.hpp"
#include "Resources/ResourcesManager.hpp"
#include "Resources/ResourceManager.hpp"
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
    , mPhysicsProxy(Physics::InvalidPhysicsProxyID)
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

PhysicsSystem* BodyComponent::GetPhysicsSystem() const
{
    Entity* entity = GetEntity();
    if (!entity)
        return nullptr;

    SceneManager* scene = entity->GetScene();
    NFE_ASSERT(scene, "Entity not attached to a scene");

    PhysicsSystem* physicsSystem = scene->GetSystem<PhysicsSystem>();
    NFE_ASSERT(physicsSystem, "Physics system is not present");

    return physicsSystem;
}

Physics::IPhysicsScene* BodyComponent::GetPhysicsScene() const
{
    PhysicsSystem* physicsSystem = GetPhysicsSystem();
    NFE_ASSERT(physicsSystem, "Physics system not found");

    return physicsSystem->GetPhysicsScene();
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

    Physics::BodyProxyInfo proxyInfo;
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

    Physics::IPhysicsScene* physicsScene = GetPhysicsScene();
    NFE_ASSERT(physicsScene, "Physics scene is not present");
    mPhysicsProxy = physicsScene->CreateBodyProxy(proxyInfo);
}

void BodyComponent::UpdatePhysicsProxy()
{
    if (mPhysicsProxy == Physics::InvalidPhysicsProxyID)
    {
        CreatePhysicsProxy();
        return;
    }

    Entity* entity = GetEntity();
    if (!GetEntity())
        return;

    Physics::BodyProxyInfo proxyInfo;
    proxyInfo.collisionShape = mCollisionShape;
    proxyInfo.mass = mMass;
    proxyInfo.transform = entity->GetGlobalTransform().ToMatrix();
    proxyInfo.inertia = Vector(1.0f, 1.0f, 1.0f); // TODO
    proxyInfo.velocity = mVelocity;
    proxyInfo.angularVelocity = mAngularVelocity;

    Physics::IPhysicsScene* physicsScene = GetPhysicsScene();
    NFE_ASSERT(physicsScene, "Physics scene is not present");
    physicsScene->UpdateBodyProxy(mPhysicsProxy, proxyInfo);
}

void BodyComponent::DeletePhysicsProxy()
{
    if (mPhysicsProxy == Physics::InvalidPhysicsProxyID)
        return;

    Physics::IPhysicsScene* physicsScene = GetPhysicsScene();
    NFE_ASSERT(physicsScene, "Physics scene is not present");
    physicsScene->DeleteBodyProxy(mPhysicsProxy);
    mPhysicsProxy = Physics::InvalidPhysicsProxyID;
}

} // namespace Scene
} // namespace NFE
