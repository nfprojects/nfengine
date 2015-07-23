/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of physics system.
 */

#include "../PCH.hpp"
#include "PhysicsSystem.hpp"
#include "../Globals.hpp"
#include "../Engine.hpp"
#include "../SceneManager.hpp"
#include "../Performance.hpp"
#include "../Entity.hpp"
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
{
    mScene = scene;

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
    Common::Timer timer;
    timer.Start();
    mDynamicsWorld->stepSimulation(dt, 40, 1.0f / 90.0f);
    Util::g_FrameStats.physics = timer.Stop();
}

void PhysicsSystem::ProcessContacts()
{
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
                // TODO: finish when event system is implemented
                //const btVector3& ptA = pt.getPositionWorldOnA();
                //const btVector3& ptB = pt.getPositionWorldOnB();
                //const btVector3& normalOnB = pt.m_normalWorldOnB;
            }
        }
    }
}

void PhysicsSystem::Update(float dt)
{
    // TODO: multithreading
    UpdatePhysics(dt);

    // TODO: use EntityManager to obtain (BodyComponent, TransformComponent) pairs
    for (auto bodyTuple : mScene->mBodies)
    {
        TransformComponent* transform = std::get<0>(bodyTuple);
        BodyComponent* body = std::get<1>(bodyTuple);

        // execute body deletion
        if (body->mFlags & BODY_COMPONENT_FLAG_RELEASE)
        {
            if (body->mRigidBody)
                mDynamicsWorld->removeRigidBody(body->mRigidBody.get());
            body->mRigidBody.reset();
            body->mMotionState.reset();
        }

        // execute body creation
        if (body->mFlags & BODY_COMPONENT_FLAG_INIT)
        {
            btVector3 inertia = btVector3(body->mCollisionShape->mLocalInertia.f[0],
                                          body->mCollisionShape->mLocalInertia.f[1],
                                          body->mCollisionShape->mLocalInertia.f[2]);

            btTransform bodyTransform;
            bodyTransform.setFromOpenGLMatrix(transform->GetMatrix().f);
            body->mMotionState.reset(new btDefaultMotionState(bodyTransform));
            body->mRigidBody.reset(new btRigidBody(body->mMass, body->mMotionState.get(),
                body->mCollisionShape->mShape, body->mMass * inertia));
            body->mRigidBody->setUserPointer(this);

            // set material parameters - TODO
            body->mRigidBody->setFriction(0.5f);
            body->mRigidBody->setRestitution(0.1f);
            body->mRigidBody->setDamping(0.01f, 0.01f);
            body->mRigidBody->setRollingFriction(0.1f);
            body->mRigidBody->setSleepingThresholds(0.01f, 0.01f);

            if (body->mMass > 0.0f)
            {
                // enable CCD
                body->mRigidBody->setContactProcessingThreshold(1e20f);
                body->mRigidBody->setCcdMotionThreshold(0.0f);
                body->mRigidBody->setCcdSweptSphereRadius(0.02f);
            }

            mDynamicsWorld->addRigidBody(body->mRigidBody.get());
        }

        // clear flags
        body->mFlags = 0;

        btRigidBody* rigidBody = body->mRigidBody.get();

        bool isActive = false;
        if (rigidBody)
        {
            isActive = rigidBody->isActive();
            if (isActive)
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
            isActive = (VectorGreaterMask(VectorAbs(body->GetVelocity()), g_Epsilon) & 0x7) != 0;
            /*
            if (isActive)
            {
                Vector pos = transform->Get.r[3];
                pos += transformComp->mVelocity * dt;
                transformComp->mMatrix.r[3] = pos;
            }
            */
        }

        if (isActive)
        {
            // TODO: entity manger should update children positions
            //transform->UpdateLocalMatrix();
        }
    }

    ProcessContacts();
}

} // namespace Scene
} // namespace NFE
