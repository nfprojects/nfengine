/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of physics system.
 */

#include "PCH.hpp"
#include "PhysicsSystemImpl.hpp"
#include "Engine.hpp"
#include "Utils/ConfigVariable.hpp"

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

void PhysicsSystem::UpdatePhysics(float dt)
{
    mDynamicsWorld->stepSimulation(dt, gMaxSubSteps.Get(), gFixedTimeStep.Get());
}

PhysicsProxyID PhysicsSystem::CreateBodyProxy(const PhysicsBodyProxyInfo& info)
{
    PhysicsBodyProxy proxy;
    proxy.info = info;
    // TODO create Bullet's objects

    return mBodyProxies.Add(std::move(proxy));
}

bool PhysicsSystem::UpdateBodyProxy(const PhysicsProxyID proxyID, const PhysicsBodyProxyInfo& info)
{
    NFE_ASSERT(mBodyProxies.Has(proxyID), "Invalid physics proxy ID");

    mBodyProxies[proxyID].info = info;
    // TODO apply changes

    return true;
}

void PhysicsSystem::DeleteBodyProxy(const PhysicsProxyID proxyID)
{
    NFE_ASSERT(mBodyProxies.Has(proxyID), "Invalid physics proxy ID");

    mBodyProxies.Remove(proxyID);
}

void PhysicsSystem::Update(float dt)
{
    /*
    // TODO: multithreading
    UpdatePhysics(dt);

    auto iterFunc = [this, dt](EntityID entity, TransformComponent* transform, BodyComponent* body)
    {
        // execute body deletion
        if (body->mFlags & BODY_COMPONENT_FLAG_RELEASE)
        {
            if (body->mRigidBody)
                mDynamicsWorld->removeRigidBody(body->mRigidBody.get());
            body->mRigidBody.reset();
            body->mMotionState.reset();

            // clear flag
            body->mFlags &= ~BODY_COMPONENT_FLAG_RELEASE;
        }

        // execute body creation
        if (body->mFlags & BODY_COMPONENT_FLAG_INIT)
        {
            btVector3 inertia = btVector3(body->mCollisionShape->mLocalInertia.f[0],
                                          body->mCollisionShape->mLocalInertia.f[1],
                                          body->mCollisionShape->mLocalInertia.f[2]);

            btCollisionShape* shape = &mEmptyCollisionShape;
            if (body->mCollisionShape != nullptr)
                if (body->mCollisionShape->mShape != nullptr)
                    shape = body->mCollisionShape->mShape;

            btTransform bodyTransform;
            bodyTransform.setFromOpenGLMatrix(transform->GetMatrix().f);
            body->mMotionState.reset(new btDefaultMotionState(bodyTransform));
            body->mRigidBody.reset(new btRigidBody(body->mMass,
                                                   body->mMotionState.get(),
                                                   shape,
                                                   body->mMass * inertia));

            // set material parameters - TODO
            body->mRigidBody->setFriction(0.5f);
            body->mRigidBody->setRestitution(0.1f);
            body->mRigidBody->setDamping(0.01f, 0.01f);
            body->mRigidBody->setRollingFriction(0.1f);
            body->mRigidBody->setSleepingThresholds(0.005f, 0.005f);

            if (body->mMass > 0.0f)
            {
                // enable CCD
                body->mRigidBody->setContactProcessingThreshold(1e20f);
                body->mRigidBody->setCcdMotionThreshold(0.0f);
                body->mRigidBody->setCcdSweptSphereRadius(0.02f);
            }

            mDynamicsWorld->addRigidBody(body->mRigidBody.get());

            // clear flag
            body->mFlags &= ~BODY_COMPONENT_FLAG_INIT;
        }


        btRigidBody* rigidBody = body->mRigidBody.get();

        bool isActive = false;
        if (rigidBody)
        {
            // entity has moved
            if (transform->mFlags & NFE_TRANSFORM_FLAG_GLOBAL_MOVED)
            {
                btTransform bodyTransform;
                bodyTransform.setFromOpenGLMatrix(transform->GetMatrix().f);
                body->mMotionState.reset(new btDefaultMotionState(bodyTransform));

                rigidBody->setMotionState(body->mMotionState.get());
                rigidBody->activate();
            }

            // get entity transform
            else if (rigidBody->isActive())
            {
                btTransform bodyTransform;
                rigidBody->getMotionState()->getWorldTransform(bodyTransform);

                Matrix matrix;
                bodyTransform.getOpenGLMatrix(matrix.f);
                transform->SetMatrix(matrix);
            }
        }
        else
        {
            isActive = (Vector::GreaterMask(Vector::Abs(body->GetVelocity()), VECTOR_EPSILON) & 0x7)
                           != 0;
            if (isActive)
            {
                Vector pos = transform->GetPosition();
                pos += body->mVelocity * dt;
                transform->SetPosition(pos);
            }
        }
    };

    // TODO: use listener instead
    mScene->GetEntityManager()->ForEach_DEPRECATED<TransformComponent, BodyComponent>(iterFunc);
    */
}

} // namespace Scene
} // namespace NFE
