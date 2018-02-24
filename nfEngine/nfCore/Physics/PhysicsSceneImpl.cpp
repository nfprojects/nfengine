/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of physics scene.
 */

#include "PCH.hpp"
#include "PhysicsSceneImpl.hpp"
#include "Engine.hpp"
#include "Utils/ConfigVariable.hpp"
#include "Resources/CollisionShape.hpp"

#include "nfCommon/System/Timer.hpp"


namespace NFE {

using namespace Math;


// size of fixed-time step (in seconds)
ConfigVariable<float> gFixedTimeStep("physics/fixedTimeStep", 1.0f / 60.0f);

// maximum number of sub-steps within a single simulation step (scene update)
ConfigVariable<int> gMaxSubSteps("physics/maxSubSteps", 50);

// enable continuous collision detection (CCD)
ConfigVariable<bool> gEnableCCD("physics/ccd", true);


namespace {

/**
* Convert Bullet's vector NFEngine's Vector4
*/
Vector4 bt2Vector(const btVector3& v)
{
    return Vector4((float*)v.m_floats);
}

/**
* Convert NFEngine's Vector to Bullet's vector4
*/
btVector3 Vector2bt(const Vector4& v)
{
    return btVector3(v.f[0], v.f[1], v.f[2]);
}

} // namespace

//////////////////////////////////////////////////////////////////////////

namespace Physics {


BodyProxy::~BodyProxy()
{
}

PhysicsScene::PhysicsScene()
{
    mBroadphase = Common::MakeUniquePtr<btDbvtBroadphase>();
    mCollsionConfig = Common::MakeUniquePtr<btDefaultCollisionConfiguration>();
    mDispatcher = Common::MakeUniquePtr<btCollisionDispatcher>(mCollsionConfig.Get());
    mSolver = Common::MakeUniquePtr<btSequentialImpulseConstraintSolver>();
    mDynamicsWorld = Common::MakeUniquePtr<btDiscreteDynamicsWorld>(mDispatcher.Get(), mBroadphase.Get(), mSolver.Get(), mCollsionConfig.Get());

    mDynamicsWorld->getDispatchInfo().m_useContinuous = gEnableCCD.Get();
    mDynamicsWorld->getSolverInfo().m_solverMode |= SOLVER_SIMD | SOLVER_USE_WARMSTARTING;
    mDynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));
}

PhysicsScene::~PhysicsScene()
{
    NFE_ASSERT(mBodyProxies.Size() == 0, "Physics proxies list is not empty. All components using physics system should be destroyed");
}

ProxyID PhysicsScene::CreateBodyProxy(const BodyProxyInfo& info)
{
    NFE_ASSERT(info.mass >= 0.0f, "Body mass cannot be negative");

    BodyProxy proxy;
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
    proxy.motionState = Common::MakeUniquePtr<btDefaultMotionState>(bodyTransform);

    // create rigid body object
    proxy.rigidBody = Common::MakeUniquePtr<btRigidBody>(info.mass, proxy.motionState.Get(), shape, info.mass * inertia);

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
    mDynamicsWorld->addRigidBody(proxy.rigidBody.Get());
    proxy.rigidBody->activate();

    return mBodyProxies.Add(std::move(proxy));
}

bool PhysicsScene::UpdateBodyProxy(const ProxyID proxyID, const BodyProxyInfo& newInfo)
{
    NFE_ASSERT(mBodyProxies.Has(proxyID), "Invalid physics proxy ID");

    BodyProxy& bodyProxy = mBodyProxies[proxyID];

    bool requiresActivation = false;

    // handle transform change
    if (!Matrix::Equal(bodyProxy.info.transform, newInfo.transform, NFE_MATH_EPSILON))
    {
        // update new motion state
        btTransform bodyTransform;
        bodyTransform.setFromOpenGLMatrix(newInfo.transform.f);
        bodyProxy.motionState = Common::MakeUniquePtr<btDefaultMotionState>(bodyTransform);

        // attach motion state
        bodyProxy.rigidBody->setMotionState(bodyProxy.motionState.Get());

        requiresActivation = true;
    }

    // handle velocity change
    if (!Vector4::AlmostEqual(bodyProxy.info.velocity, newInfo.velocity))
    {
        bodyProxy.info.velocity = newInfo.velocity;
        bodyProxy.rigidBody->setLinearVelocity(Vector2bt(newInfo.velocity));
        requiresActivation = true;
    }

    // handle angular velocity change
    if (!Vector4::AlmostEqual(bodyProxy.info.angularVelocity, newInfo.angularVelocity))
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

void PhysicsScene::DeleteBodyProxy(const ProxyID proxyID)
{
    // remove body from the physics world
    BodyProxy& bodyProxy = mBodyProxies[proxyID];
    mDynamicsWorld->removeRigidBody(bodyProxy.rigidBody.Get());

    // TODO memory leak?
    mBodyProxies.Remove(proxyID);
}

void PhysicsScene::Update(float dt)
{
    // TODO multithreading !!!

    mDynamicsWorld->stepSimulation(dt, gMaxSubSteps.Get(), gFixedTimeStep.Get());

    // TODO iterate only active proxies (use physics engine events?)
    mBodyProxies.Iterate([this](BodyProxy& proxy)
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
                const Vector4 velocity = bt2Vector(proxy.rigidBody->getLinearVelocity());
                const Vector4 angularVelocity = bt2Vector(proxy.rigidBody->getAngularVelocity());

                proxy.info.velocity = velocity;
                proxy.info.angularVelocity = angularVelocity;

                proxy.info.velocityUpdateCallback(velocity, angularVelocity);
            }
        }
    });
}

} // namespace Physics
} // namespace NFE
