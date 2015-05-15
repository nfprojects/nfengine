/**
 * @file   Body.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Body component definition.
*/

#include "PCH.hpp"
#include "Globals.hpp"
#include "Body.hpp"
#include "SceneManager.hpp"
#include "PhysicsWorld.hpp"
#include "ResourcesManager.hpp"
#include "CollisionShape.hpp"
#include "Entity.hpp"
#include "..\nfCommon\InputStream.hpp"
#include "..\nfCommon\OutputStream.hpp"

namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;

BodyComponent::BodyComponent(Entity* pParent) : Component(pParent)
{
    mType = ComponentType::Physics;

    mBody = 0;
    mCollisionShape = 0;
    mMotionState = 0;

    mMass = 0.0f;
}

BodyComponent::~BodyComponent()
{
    DisablePhysics();

    if (mCollisionShape)
    {
        mCollisionShape->DelRef();
        mCollisionShape = 0;
    }
}

btVector3 XVec2bt(const Vector& v)
{
    return btVector3(v.f[0], v.f[1], v.f[2]);
}

Vector bt2XVec(const btVector3& v)
{
    return Vector((float*)v.m_floats);
}

//1. update position (if physics is enabled)
//2. update children
void BodyComponent::OnUpdate(float dt)
{
    bool isActive = false;

    if (mBody)
    {
        isActive = mBody->isActive();
        if (isActive)
        {
            btTransform transform;
            mBody->getMotionState()->getWorldTransform(transform);
            transform.getOpenGLMatrix(mOwner->mMatrix.f);

            mOwner->mVelocity = bt2XVec(mBody->getLinearVelocity());
            mOwner->mAngularVelocity = bt2XVec(mBody->getAngularVelocity());
        }
    }
    else
    {
        isActive = (VectorGreaterMask(VectorAbs(mOwner->mVelocity), g_Epsilon) & 0x7) != 0;
        if (isActive)
        {
            Vector pos = mOwner->mMatrix.r[3];
            pos += mOwner->mVelocity * dt;
            mOwner->mMatrix.r[3] = pos;
        }
    }

    if (isActive)
    {
        mOwner->UpdateLocalMatrix();
        mOwner->OnMove();
    }
}

void BodyComponent::OnMove()
{
    if (mBody && mMotionState)
    {
        /*
        btTransform transform;
        transform.setFromOpenGLMatrix(mLocalMatrix.f);
        mBody->getMotionState()->setWorldTransform(transform);
        */

        btTransform transform;
        transform.setFromOpenGLMatrix(mOwner->mMatrix.f);
        btDefaultMotionState* pNewMotionState = new btDefaultMotionState(transform);

        mBody->setMotionState(pNewMotionState);
        delete mMotionState;
        mMotionState = pNewMotionState;
    }
}

void BodyComponent::EnablePhysics(CollisionShape* pCollisionShape)
{
    if (mBody)
        return;

    if (mCollisionShape)
        mCollisionShape->DelRef();

    mCollisionShape = pCollisionShape;
    mCollisionShape->AddRef();


    btVector3 inertia = btVector3(mCollisionShape->mLocalInertia.f[0],
                                  mCollisionShape->mLocalInertia.f[1],
                                  mCollisionShape->mLocalInertia.f[2]);

    btTransform transform;
    transform.setFromOpenGLMatrix(mOwner->mMatrix.f);
    mMotionState = new btDefaultMotionState(transform);
    mBody = new btRigidBody(mMass, mMotionState, mCollisionShape->mShape, mMass * inertia);
    mBody->setUserPointer(this);

    // set material parameters - NEEDS CHANGE
    mBody->setFriction(0.5f);
    mBody->setRestitution(0.1f);
    mBody->setDamping(0.01f, 0.01f);
    mBody->setRollingFriction(0.1f);
    mBody->setSleepingThresholds(0.01f, 0.01f);


    if (mMass > 0.0f)
    {
        // enable CCD
        mBody->setContactProcessingThreshold(1e20f);
        mBody->setCcdMotionThreshold(0.0f);
        mBody->setCcdSweptSphereRadius(0.02f);
    }


    //set velocities
    mBody->setLinearVelocity(XVec2bt(mOwner->mVelocity));
    mBody->setAngularVelocity(XVec2bt(mOwner->mAngularVelocity));

    // TODO
    if (mOwner->mScene != NULL)
    {
        mOwner->mScene->SyncPhysics();
        mOwner->mScene->mWorld->mDynamicsWorld->addRigidBody(mBody);
    }
}

void BodyComponent::DisablePhysics()
{
    if (mBody == 0)
        return;

    if (mOwner)
        if (mOwner->mScene)
            mOwner->mScene->mWorld->mDynamicsWorld->removeRigidBody(mBody);

    delete mBody->getMotionState();
    delete mBody;

    mBody = 0;
    mMotionState = 0;
}

/*
void BodyComponent::SetVelocity(const Vector& velocity)
{
    mVelocity = velocity;
    Move();

    if (mBody)
        mBody->setLinearVelocity(XVec2bt(velocity));
}

void BodyComponent::SetAngularVelocity(const Vector& velocity)
{
    mAngularVelocity = velocity;
    Move();

    if (mBody)
        mBody->setAngularVelocity(XVec2bt(velocity));
}

Vector BodyComponent::GetVelocity()
{
    return mVelocity;
}

Vector BodyComponent::GetAngularVelocity()
{
    return mAngularVelocity;
}
*/

void BodyComponent::SetMass(float mass)
{
    mMass = mass;

    if (mBody && mCollisionShape)
    {
        btVector3 inertia = btVector3(mCollisionShape->mLocalInertia.f[0],
                                      mCollisionShape->mLocalInertia.f[1],
                                      mCollisionShape->mLocalInertia.f[2]);

        mBody->setMassProps(mMass, inertia * mMass);
    }
}

float BodyComponent::GetMass()
{
    return mMass;
}


Result BodyComponent::Deserialize(Common::InputStream* pStream)
{
    BodyComponentDesc desc;
    if (pStream->Read(sizeof(desc), &desc) != sizeof(desc))
        return Result::Error;

    mMass = desc.mass;

    CollisionShape* pShape = (CollisionShape*)g_pResManager->GetResource(desc.collisionShapeName,
                             ResourceType::CollisionShape);
    EnablePhysics(pShape);

    return Result::OK;
}

Result BodyComponent::Serialize(Common::OutputStream* pStream) const
{
    BodyComponentDesc desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.mass = mMass;

    if (mCollisionShape)
        strcpy(desc.collisionShapeName, mCollisionShape->GetName());

    if (pStream->Write(&desc, sizeof(desc)) != sizeof(desc))
        return Result::Error;

    return Result::OK;
}

void BodyComponent::ReceiveMessage(ComponentMsg type, void* pData)
{
    switch (type)
    {
        case ComponentMsg::OnMove:
            OnMove();
            break;

        case ComponentMsg::OnSetVelocity:
            if (mBody)
            {
                mBody->setLinearVelocity(XVec2bt(mOwner->mVelocity));
                mBody->activate(true);
            }
            break;

        case ComponentMsg::OnSetAngularVelocity:
            if (mBody)
            {
                mBody->setAngularVelocity(XVec2bt(mOwner->mAngularVelocity));
                mBody->activate(true);
            }
            break;
    }
}

} // namespace Scene
} // namespace NFE
