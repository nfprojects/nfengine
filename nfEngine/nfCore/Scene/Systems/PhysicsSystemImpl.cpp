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


// TODO remove
/*
PhysicsBodyProxy::~PhysicsBodyProxy()
{
    LOG_INFO("PhysicsBodyProxy::~PhysicsBodyProxy()");
}

PhysicsBodyProxy::PhysicsBodyProxy(PhysicsBodyProxy&& rhs)
    : info(rhs.info)
    , motionState(std::move(rhs.motionState))
    , rigidBody(std::move(rhs.rigidBody))
{
}

PhysicsBodyProxy& PhysicsBodyProxy::operator=(PhysicsBodyProxy&& rhs)
{
    info = rhs.info;
    motionState = std::move(rhs.motionState);
    rigidBody = std::move(rhs.rigidBody);
    return *this;
}
*/

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

PhysicsSystem::~PhysicsSystem()
{
    NFE_ASSERT(mBodyProxies.Size() == 0, "Physics proxies list is not empty. All components using physics system should be destroyed");
}

PhysicsProxyID PhysicsSystem::CreateBodyProxy(const PhysicsBodyProxyInfo& info)
{
    NFE_ASSERT(info.mass >= 0.0f, "Body mass cannot be negative");

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

    // TODO
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

bool PhysicsSystem::UpdateBodyProxy(const PhysicsProxyID proxyID, const PhysicsBodyProxyInfo& newInfo)
{
    NFE_ASSERT(mBodyProxies.Has(proxyID), "Invalid physics proxy ID");

    PhysicsBodyProxy& bodyProxy = mBodyProxies[proxyID];

    bool requiresActivation = false;

    // handle transform change
    if (!Matrix::Equal(bodyProxy.info.transform, newInfo.transform, NFE_MATH_EPSILON))
    {
        // update new motion state
        btTransform bodyTransform;
        bodyTransform.setFromOpenGLMatrix(newInfo.transform.f);
        bodyProxy.motionState.reset(new btDefaultMotionState(bodyTransform));

        // attach motion state
        bodyProxy.rigidBody->setMotionState(bodyProxy.motionState.get());

        requiresActivation = true;
    }

    // handle velocity change
    if (!Vector::AlmostEqual(bodyProxy.info.velocity, newInfo.velocity))
    {
        bodyProxy.info.velocity = newInfo.velocity;
        bodyProxy.rigidBody->setLinearVelocity(Vector2bt(newInfo.velocity));
        requiresActivation = true;
    }

    // handle angular velocity change
    if (!Vector::AlmostEqual(bodyProxy.info.angularVelocity, newInfo.angularVelocity))
    {
        bodyProxy.info.angularVelocity = newInfo.angularVelocity;
        bodyProxy.rigidBody->setAngularVelocity(Vector2bt(newInfo.angularVelocity));
        requiresActivation = true;
    }

    // TODO handle other parameters change

    if (requiresActivation)
    {
        bodyProxy.rigidBody->activate();
    }

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

    // TODO iterate only active proxies (use physics engine events?)
    mBodyProxies.Iterate([this](PhysicsBodyProxy& proxy)
    {
        if (proxy.rigidBody->isActive())
        {
            if (proxy.info.transformUpdateCallback)
            {
                btTransform bodyTransform;
                proxy.rigidBody->getMotionState()->getWorldTransform(bodyTransform);

                Matrix matrix;
                bodyTransform.getOpenGLMatrix(matrix.f);
                proxy.info.transform = matrix;

                proxy.info.transformUpdateCallback(matrix);
            }

            if (proxy.info.velocityUpdateCallback)
            {
                const Vector velocity = bt2Vector(proxy.rigidBody->getLinearVelocity());
                const Vector angularVelocity = bt2Vector(proxy.rigidBody->getAngularVelocity());

                proxy.info.velocity = velocity;
                proxy.info.angularVelocity = angularVelocity;

                proxy.info.velocityUpdateCallback(velocity, angularVelocity);
            }
        }
    });
}

} // namespace Scene
} // namespace NFE
