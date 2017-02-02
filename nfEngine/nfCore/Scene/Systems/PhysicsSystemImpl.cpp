/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of physics system.
 */

#include "PCH.hpp"
#include "PhysicsSystemImpl.hpp"
#include "Engine.hpp"
#include "Utils/ConfigVariable.hpp"
#include "Resources/CollisionShape.hpp"

#include "nfCommon/System/Timer.hpp"


namespace NFE {


ConfigVariable<float> gFixedTimeStep("physics/fixedTimeStep", 1.0f / 60.0f);
ConfigVariable<int> gMaxSubSteps("physics/maxSubSteps", 50);
ConfigVariable<bool> gEnableCCD("physics/ccd", true);


namespace Scene {

using namespace Math;

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

PhysicsSystem::PhysicsSystem(SceneManager* scene)
    : IPhysicsSystem(scene)
{
    mBroadphase.reset(new btDbvtBroadphase());
    mCollsionConfig.reset(new btDefaultCollisionConfiguration());
    mDispatcher.reset(new btCollisionDispatcher(mCollsionConfig.get()));
    mSolver.reset(new btSequentialImpulseConstraintSolver);
    mDynamicsWorld.reset(new btDiscreteDynamicsWorld(mDispatcher.get(), mBroadphase.get(), mSolver.get(), mCollsionConfig.get()));

    mDynamicsWorld->getDispatchInfo().m_useContinuous = gEnableCCD.Get();
    mDynamicsWorld->getSolverInfo().m_solverMode |= SOLVER_SIMD | SOLVER_USE_WARMSTARTING;
    mDynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));
}

PhysicsProxyID PhysicsSystem::CreateBodyProxy(const PhysicsBodyProxyInfo& info)
{
    PhysicsBodyProxy proxy;
    proxy.info = info;

    btVector3 inertia;
    btCollisionShape* shape = &mEmptyCollisionShape;
    if (info.collisionShape)
    {
        if (info.collisionShape->GetShape())
        {
            inertia = Vector2bt(info.collisionShape->GetLocalInertia());
            shape = info.collisionShape->GetShape();
        }
    }

    // setup motion state
    btTransform bodyTransform;
    bodyTransform.setFromOpenGLMatrix(info.transform.f);
    proxy.motionState.reset(new btDefaultMotionState(bodyTransform));

    // create rigid body object
    proxy.rigidBody.reset(new btRigidBody(info.mass, proxy.motionState.get(), shape, info.mass * inertia));

    // set material parameters - TODO
    proxy.rigidBody->setFriction(0.5f);
    proxy.rigidBody->setRestitution(0.1f);
    proxy.rigidBody->setDamping(0.01f, 0.01f);
    proxy.rigidBody->setRollingFriction(0.1f);
    proxy.rigidBody->setSleepingThresholds(0.005f, 0.005f);

    if (info.mass > 0.0f)
    {
        // enable CCD
        proxy.rigidBody->setContactProcessingThreshold(1e20f);
        proxy.rigidBody->setCcdMotionThreshold(0.0f);
        proxy.rigidBody->setCcdSweptSphereRadius(0.02f);
    }

    // attach body to physics world
    mDynamicsWorld->addRigidBody(proxy.rigidBody.get());
    proxy.rigidBody->activate();

    return mBodyProxies.Add(std::move(proxy));
}

bool PhysicsSystem::UpdateBodyProxy(const PhysicsProxyID proxyID, const PhysicsBodyProxyInfo& info)
{
    NFE_ASSERT(mBodyProxies.Has(proxyID), "Invalid physics proxy ID");

    PhysicsBodyProxy& bodyProxy = mBodyProxies[proxyID];
    bodyProxy.info = info;

    // update new motion state
    btTransform bodyTransform;
    bodyTransform.setFromOpenGLMatrix(info.transform.f);
    bodyProxy.motionState.reset(new btDefaultMotionState(bodyTransform));

    // attach motion state
    bodyProxy.rigidBody->setMotionState(bodyProxy.motionState.get());

    bodyProxy.rigidBody->activate();

    return true;
}

void PhysicsSystem::DeleteBodyProxy(const PhysicsProxyID proxyID)
{
    NFE_ASSERT(mBodyProxies.Has(proxyID), "Invalid physics proxy ID");

    // remove body from the physics world
    PhysicsBodyProxy& bodyProxy = mBodyProxies[proxyID];
    mDynamicsWorld->removeRigidBody(bodyProxy.rigidBody.get());

    // TODO memory leak?
    mBodyProxies.Remove(proxyID);
}

void PhysicsSystem::Update(float dt)
{
    // TODO multithreading !!!

    mDynamicsWorld->stepSimulation(dt, gMaxSubSteps.Get(), gFixedTimeStep.Get());

    // TODO iterate only dynamic proxies
    mBodyProxies.Iterate([this](const PhysicsBodyProxy& proxy)
    {
        if (proxy.rigidBody->isActive() && proxy.info.updateCallback)
        {
            btTransform bodyTransform;
            proxy.rigidBody->getMotionState()->getWorldTransform(bodyTransform);

            Matrix matrix;
            bodyTransform.getOpenGLMatrix(matrix.f);

            proxy.info.updateCallback(matrix);
        }
    });
}

} // namespace Scene
} // namespace NFE
