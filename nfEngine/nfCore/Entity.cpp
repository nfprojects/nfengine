/**
    NFEngine project

    \file   Entity.cpp
    \brief  Scene entity definition.
*/

#include "stdafx.hpp"
#include "Entity.hpp"
#include "SceneManager.hpp"
#include "Memory.hpp"
#include "../nfCommon/InputStream.hpp"
#include "../nfCommon/OutputStream.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

Entity::Entity()
{
    mLocalMatrix = mMatrix = MatrixIdentity();
    mVelocity = mAngularVelocity = Vector();

    mFlags = 0;
    mScene = nullptr;
    mSegment = nullptr;
    mUserPointer = nullptr;
    mParent = nullptr;
    mName = nullptr;
}

/*
    Clean up children / parent connections. Delete components
*/
Entity::~Entity()
{
    if (mParent)
        mParent->Detach(this);

    RemoveAllComponents();

    //remove child entities
    for (auto pChild : mChildren) pChild->mParent =
            nullptr; // TODO: switch child's parent to a grandparnet
    mChildren.clear();

    if (mName)
        free(mName);
}


Result Entity::SetName(const char* pNewName)
{
    // TODO: validate input, check if a name is not used
    mName = strdup(pNewName);
    return Result::OK;
}

const char* Entity::GetName() const
{
    return mName;
}

SceneManager* Entity::GetScene() const
{
    return mScene;
}

Segment* Entity::GetSceneSegment() const
{
    return mSegment;
}

Entity* Entity::GetParent() const
{
    return mParent;
}

void Entity::SetUserPointer(void* ptr)
{
    mUserPointer = ptr;
}

void* Entity::GetUserPointer() const
{
    return mUserPointer;
}

Result Entity::Attach(Entity* pChild)
{
#ifdef _DEBUG // check if pointer is valid
    if (!(Util::MemoryCheck(pChild) & (ACCESS_READ | ACCESS_WRITE)))
    {
        LOG_ERROR("Invalid entity pointer (%p)", pChild);
        return Result::CorruptedPointer;
    }
#endif

    if (pChild->mParent == this)
        return Result::OK;
    else if (pChild->mParent != nullptr)
    {
        // TODO: Detach
        //pChild->mParent->D
    }

    mChildren.insert(pChild);
    pChild->mParent = this;

    return Result::OK;
}

Result Entity::Detach(Entity* pChild)
{
#ifdef _DEBUG // check if pointer is valid
    if (!(Util::MemoryCheck(pChild) & (ACCESS_READ | ACCESS_WRITE)))
    {
        LOG_ERROR("Invalid entity pointer (%p)", pChild);
        return Result::CorruptedPointer;
    }
#endif

    if (pChild->mParent == 0)
        return Result::OK;
    else if (pChild->mParent != this)
    {
        LOG_ERROR("This is not a child entity. Perform Detach() on parent entity", pChild);
        Result::Error;
    }

    mChildren.erase(pChild);
    pChild->mParent = nullptr;

    return Result::OK;
}


void Entity::UpdateLocalMatrix()
{
    if (mParent != nullptr)
        mLocalMatrix = mMatrix * MatrixInverse(mParent->mMatrix);
    else
        mLocalMatrix = mMatrix;
}

void Entity::UpdateGlobalMatrix()
{
    if (mParent != nullptr)
        mMatrix = mLocalMatrix * mParent->mMatrix;
    else
        mMatrix = mLocalMatrix;
}

//
// GLOBAL SPACE
//

void Entity::SetMatrix(const Matrix& matrix)
{
    mFlags |= ENTITY_FLAG_MOVED;
    mMatrix = matrix;

    UpdateLocalMatrix();
    OnMove();
}

Matrix Entity::GetMatrix() const
{
    return mMatrix;
}

void Entity::SetPosition(const Vector& pos)
{
    mFlags |= ENTITY_FLAG_MOVED;

    mMatrix.r[3] = pos;
    mMatrix.r[3].m128_f32[3] = 1.0f;

    UpdateLocalMatrix();
    OnMove();
}

Vector Entity::GetPosition() const
{
    return mMatrix.r[3];
}

void Entity::SetOrientation(const Quaternion& quat)
{
    mFlags |= ENTITY_FLAG_MOVED;


    Matrix rotMatrix = MatrixFromQuaternion(QuaternionNormalize(quat));
    mMatrix.r[0] = rotMatrix.r[0];
    mMatrix.r[1] = rotMatrix.r[1];
    mMatrix.r[2] = rotMatrix.r[2];

    UpdateLocalMatrix();
    OnMove();
}

void Entity::SetOrientation(const XOrientation* pOrientation)
{
    mFlags |= ENTITY_FLAG_MOVED;

    //normalize directions
    Vector zAxis, yAxis, xAxis;
    zAxis = VectorNormalize3(pOrientation->z);
    xAxis = VectorNormalize3(VectorCross3(pOrientation->y, pOrientation->z));
    yAxis = VectorNormalize3(VectorCross3(zAxis, xAxis));

    mMatrix.r[0] = xAxis;
    mMatrix.r[1] = yAxis;
    mMatrix.r[2] = zAxis;

    mMatrix.r[0].m128_f32[3] = 0.0f;
    mMatrix.r[1].m128_f32[3] = 0.0f;
    mMatrix.r[2].m128_f32[3] = 0.0f;

    UpdateLocalMatrix();
    OnMove();
}

void Entity::GetOrientation(XOrientation* pOrientation) const
{
    pOrientation->x = mMatrix.r[0];
    pOrientation->y = mMatrix.r[1];
    pOrientation->z = mMatrix.r[2];
}

//
// LOCAL SPACE
//

void Entity::SetLocalPosition(const Vector& pos)
{
    mLocalMatrix.r[3] = pos;
    mLocalMatrix.r[3].m128_f32[3] = 1.0f;

    UpdateGlobalMatrix();
    OnMove();
}

Vector Entity::GetLocalPosition() const
{
    return mLocalMatrix.r[3];
}

void Entity::SetLocalOrientation(const XOrientation* pOrientation)
{
    //normalize directions
    Vector zAxis, yAxis, xAxis;
    zAxis = VectorNormalize3(pOrientation->z);
    xAxis = VectorNormalize3(VectorCross3(pOrientation->y, pOrientation->z));
    yAxis = VectorNormalize3(VectorCross3(zAxis, xAxis));

    mLocalMatrix.r[0] = xAxis;
    mLocalMatrix.r[1] = yAxis;
    mLocalMatrix.r[2] = zAxis;

    mLocalMatrix.r[0].m128_f32[3] = 0.0f;
    mLocalMatrix.r[1].m128_f32[3] = 0.0f;
    mLocalMatrix.r[2].m128_f32[3] = 0.0f;

    UpdateGlobalMatrix();
    OnMove();
}

void Entity::GetLocalOrientation(XOrientation* pOrientation) const
{
    pOrientation->x = mLocalMatrix.r[0];
    pOrientation->y = mLocalMatrix.r[1];
    pOrientation->z = mLocalMatrix.r[2];
}

void Entity::SetVelocity(const Vector& newVelocity)
{
    mVelocity = newVelocity;
    BroadcastMessage(ComponentMsg::OnSetVelocity, 0);
}

void Entity::SetAngularVelocity(const Vector& newAngularVelocity)
{
    mAngularVelocity = newAngularVelocity;
    BroadcastMessage(ComponentMsg::OnSetAngularVelocity, 0);
}

Vector Entity::GetVelocity() const
{
    return mVelocity;
}

Vector Entity::GetAngularVelocity() const
{
    return mAngularVelocity;
}

void Entity::OnMove()
{
    // switch segment (if needed)
    if (mScene && (GetParent() == nullptr))
        mScene->UpdateSegmentForEntity(this);

    for (auto pComp : mComponents)
        pComp->ReceiveMessage(ComponentMsg::OnMove, 0);

    for (auto child : mChildren)
    {
        child->mSegment = mSegment; // propagate the scene segment
        child->mMatrix = child->mLocalMatrix * mMatrix;
        child->OnMove();
    }
}

// Called by the engine when the entity is spawned due to scene segment creation.
Result Entity::Deserialize(Common::InputStream* pStream, const Vector& offset)
{
    if (!pStream)
        return Result::CorruptedPointer;

    EntityDesc desc;

    size_t bytesRead = pStream->Read(sizeof(desc), &desc);
    if (bytesRead != sizeof(desc))
    {
        if (bytesRead > 0)
        {
            LOG_ERROR("Could not deserialize entity.");
            return Result::Error; //failed to read entity descriptor
        }
        else
        {
            // End of Stream. This sould be checked outside to aviod pointless Entity object creation.
            return Result::Error;
        }
    }


    // decode rotation matrix
    mLocalMatrix = MatrixFromQuaternion(Vector(desc.quat));

    // decode position
    mLocalMatrix.r[3] = Vector(desc.pos) + offset;
    mLocalMatrix.m[3][3] = 1.0f;

    UpdateGlobalMatrix();

    // deserialize components
    for (uint16 i = 0; i < desc.compNum; i++)
    {
        char compType = 0;
        if (pStream->Read(sizeof(compType), &compType) != sizeof(compType))
        {
            DebugBreak();
            LOG_ERROR("Entity deserialization failed: invalid component type. Data could be corrupted.");
            return Result::Error; //failed to component type
        }

        Component* pComponent = nullptr;
        switch ((ComponentType)compType)
        {
            case ComponentType::Mesh:
                pComponent = new MeshComponent(this);
                break;

            case ComponentType::Light:
                pComponent = new LightComponent(this);
                break;

            case ComponentType::Physics:
                pComponent = new BodyComponent(this);
                break;

            case ComponentType::Camera:
                pComponent = new Camera(this);
                break;
        }

        if (pComponent == nullptr)
        {
            LOG_ERROR("Entity deserialization failed: allocation error.");
            return Result::AllocationError; //invalid component type
        }

        pComponent->Deserialize(pStream);
    }

    // deserialize children
    for (uint16 i = 0; i < desc.childrenNum; i++)
    {
        Entity* pChild = nullptr;
        if (mScene->DeserializeEntity(pStream, Vector(), &pChild) != Result::OK)
        {
            return Result::Error;
        }

        Attach(pChild);
    }

    return Result::OK;
}

// Called by the engine when the entity is removed due to scene segment deletion.
Result Entity::Serialize(Common::OutputStream* pStream, const Vector& offset) const
{
    if (!pStream)
        return Result::CorruptedPointer;

    Quaternion quat = QuaternionFromMatrix(mLocalMatrix);

    // count non-virtual components
    uint16 compNum = 0;
    for (auto pComp : mComponents)
    {
        if (pComp == nullptr) continue;
        if (pComp->mVirtual) continue; // omit virtual components
        compNum++;
    }

    //encode local matrix
    EntityDesc desc;
    VectorStore(GetLocalPosition() - offset, &desc.pos);
    VectorStore(quat, &desc.quat);
    desc.compNum = compNum;
    desc.childrenNum = (uint16)mChildren.size();
    pStream->Write(&desc, sizeof(desc));

    // serialize components
    for (auto pComp : mComponents)
    {
        if (pComp == nullptr) continue;
        if (pComp->mVirtual) continue; // omit virtual components

        char compType = (char)pComp->mType;
        pStream->Write(&compType, sizeof(compType));

        if (pComp->Serialize(pStream) != Result::OK)
        {
            LOG_ERROR("Entity serialization failed.");
            return Result::Error;
        }
    }

    // serialize child entities
    for (auto pChild : mChildren)
    {
        if (pChild->Serialize(pStream, Vector()) != Result::OK)
        {
            LOG_ERROR("Entity serialization failed.");
            return Result::Error;
        }
    }

    return Result::OK;
}

Result Entity::AddComponent(Component* pComponent)
{
    if (pComponent->mOwner != nullptr)
    {
        LOG_ERROR("Component is already attached to an entity!");
        return Result::Error;
    }

    for (auto& pCurrComponent : mComponents)
    {
        if (pCurrComponent->mType == pComponent->mType)
        {
            LOG_ERROR("Component of this type is already attached!");
            return Result::Error;
        }
    }

    mComponents.insert(pComponent);
    pComponent->mOwner = this;
    return Result::OK;
}

Result Entity::RemoveComponent(Component* pComponent)
{
    if (mComponents.count(pComponent) == 0)
    {
        LOG_ERROR("Component is not attached to the entity!");
        return Result::Error;
    }

    //pComponent->mOwner = nullptrptr;
    mComponents.erase(pComponent);
    delete pComponent;
    return Result::OK;
}

Component* Entity::GetComponent(ComponentType type) const
{
    for (auto pComponent : mComponents)
        if (pComponent->mType == type)
            return pComponent;

    // component not found
    return nullptr;
}

void Entity::RemoveAllComponents()
{
    for (auto pComp : mComponents)
        delete pComp;

    mComponents.clear();
}

void Entity::BroadcastMessage(ComponentMsg type, void* pData)
{
    for (auto pComp : mComponents)
        pComp->ReceiveMessage(type, pData);
}

} // namespace Scene
} // namespace NFE
