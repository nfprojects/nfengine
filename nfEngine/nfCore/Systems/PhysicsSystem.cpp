/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of physics system.
 */

#include "PCH.hpp"
#include "PhysicsSystem.hpp"
#include "Engine.hpp"
#include "Scene/EntityManager.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/BodyComponent.hpp"

#include "../nfCommon/Timer.hpp"

namespace NFE {
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
    : mScene(scene)
{
    mBroadphase.reset(new btDbvtBroadphase());
    mCollsionConfig.reset(new btDefaultCollisionConfiguration());
    mDispatcher.reset(new btCollisionDispatcher(mCollsionConfig.get()));
    mSolver.reset(new btSequentialImpulseConstraintSolver);
    mDynamicsWorld.reset(new btDiscreteDynamicsWorld(mDispatcher.get(), mBroadphase.get(),
                                                     mSolver.get(), mCollsionConfig.get()));

    mDynamicsWorld->getDispatchInfo().m_useContinuous = true;
    mDynamicsWorld->getSolverInfo().m_solverMode |= SOLVER_SIMD | SOLVER_USE_WARMSTARTING;
    mDynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));
}

void PhysicsSystem::UpdatePhysics(float dt)
{
    mDynamicsWorld->stepSimulation(dt, 40, 1.0f / 90.0f);
}

void PhysicsSystem::ProcessContacts()
{
    // TODO: finish when event system is implemented
    /*
    EventBodyCollide event;

    // process contacts
    int numManifolds = mDispatcher->getNumManifolds();
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold = mDispatcher->getManifoldByIndexInternal(i);
        const btCollisionObject* obA = contactManifold->getBody0();
        const btCollisionObject* obB = contactManifold->getBody1();

        // extract body objects
        event.bodyA = (BodyComponent*)obA->getUserPointer();
        event.bodyB = (BodyComponent*)obB->getUserPointer();

        int numContacts = contactManifold->getNumContacts();
        for (int j = 0; j < numContacts; j++)
        {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            if (pt.getDistance() < 0.0f)
            {
                const btVector3& ptA = pt.getPositionWorldOnA();
                const btVector3& ptB = pt.getPositionWorldOnB();
                const btVector3& normalOnB = pt.m_normalWorldOnB;
            }
        }
    }
    */
}

void PhysicsSystem::Update(float dt)
{
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
                    shape = body->mCollisionShape->mShape.get();

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
            isActive = (VectorGreaterMask(VectorAbs(body->GetVelocity()), NFE_MATH_EPSILON) & 0x7)
                           != 0;
            if (isActive)
            {
                Vector pos = transform->GetPosition();
                pos += body->mVelocity * dt;
                transform->SetPosition(pos);
            }
        }
    };

    mScene->GetEntityManager()->ForEach<TransformComponent, BodyComponent>(iterFunc);

    ProcessContacts();
}

} // namespace Scene
} // namespace NFE
