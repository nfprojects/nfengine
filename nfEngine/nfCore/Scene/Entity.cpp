/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Entity class.
 */

#include "PCH.hpp"
#include "Entity.hpp"
#include "SceneManager.hpp"
#include "Components/Component.hpp"
#include "Systems/EntitySystem.hpp"

#include "nfCommon/System/Assertion.hpp"


namespace NFE {
namespace Scene {

using namespace Math;

Entity::Entity(EntityID id, SceneManager* parentScene, Entity* parentEntity, GameObjectInstance* gameObjectInstance)
    : mParent(parentEntity)
    , mScene(parentScene)
    , mGameObjectInstance(gameObjectInstance)
    , mID(id)
    , mFlags(0)
{
    NFE_ASSERT(parentScene, "Invalid entity scene");
}

Entity::~Entity()
{
    NFE_ASSERT(mParent == nullptr, "Destroyed entity has still a parent");

    DetachComponents();
}

void Entity::Remove(bool removeChildren)
{
    if (HasBeenRemoved())
        return;

    MarkAsDirty(NFE_ENTITY_FLAG_TO_REMOVE);

    if (removeChildren)
    {
        for (Entity* child : mChildren)
        {
            child->Remove();
        }
    }

    DetachComponents();
}

bool Entity::AddComponent(ComponentPtr&& component)
{
    mComponents.push_back(std::move(component));

    const ComponentPtr& attachedComponent = mComponents.back();
    attachedComponent->mEntity = this;
    attachedComponent->OnAttach();
    attachedComponent->OnUpdate();

    return true;
}

bool Entity::RemoveComponent(IComponent* component)
{
    for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    {
        if (iter->get() == component)
        {
            component->OnDetach();
            component->mEntity = nullptr;

            mComponents.erase(iter);
            return true;
        }
    }

    return false;
}

void Entity::DetachComponents()
{
    for (const ComponentPtr& component : mComponents)
    {
        if (!component->GetEntity())
            continue;

        component->OnDetach();
        component->mEntity = nullptr;
    }
}

bool Entity::Attach(Entity* child)
{
    NFE_ASSERT(child->GetParent() == nullptr, "Cannot attach entity that has a parent");
    NFE_ASSERT(child->GetScene() == GetScene(), "Child entity must belong to the same scene");

    mChildren.push_back(child);
    child->mParent = this;
    child->MarkAsDirty(NFE_ENTITY_FLAG_MOVED_GLOBAL); // so local transform gets updated

    // TODO notify Game Object Instance

    return true;
}

bool Entity::Detach(Entity* child)
{
    // TODO optimize
    auto iter = std::find(mChildren.begin(), mChildren.end(), child);
    NFE_ASSERT(iter != mChildren.end(), "Child entity not found");

    child->mParent = nullptr;
    mChildren.erase(iter);

    // TODO notify Game Object Instance

    return true;
}

bool Entity::SetParent(Entity* newParent)
{
    // TODO
    return false;
}

Matrix Entity::GetGlobalOrientation() const
{
    Matrix result = mGlobalMatrix;
    result[3] = VECTOR_IDENTITY_ROW_3;
    return result;
}

Matrix Entity::GetLocalOrientation() const
{
    Matrix result = mLocalMatrix;
    result[3] = VECTOR_IDENTITY_ROW_3;
    return result;
}

void Entity::SetGlobalMatrix(const Matrix& matrix)
{
    if (matrix == mGlobalMatrix)
        return;

    mGlobalMatrix = matrix;

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_GLOBAL);
}

void Entity::SetGlobalPosition(const Vector& pos)
{
    mGlobalMatrix.r[3] = pos;
    mGlobalMatrix.r[3].f[3] = 1.0f;

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_GLOBAL);
}

void Entity::SetGlobalOrientation(const Quaternion& quat)
{
    const Matrix rotMatrix = QuaternionToMatrix(QuaternionNormalize(quat));
    mGlobalMatrix.r[0] = rotMatrix.r[0];
    mGlobalMatrix.r[1] = rotMatrix.r[1];
    mGlobalMatrix.r[2] = rotMatrix.r[2];

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_GLOBAL);
}

void Entity::SetGlobalOrientation(const Matrix& orientation)
{
    // normalize directions
    const Vector zAxis = VectorNormalize3(orientation[2]);
    const Vector xAxis = VectorNormalize3(VectorCross3(orientation[1], orientation[2]));
    const Vector yAxis = VectorNormalize3(VectorCross3(zAxis, xAxis));

    mGlobalMatrix.r[0] = xAxis;
    mGlobalMatrix.r[1] = yAxis;
    mGlobalMatrix.r[2] = zAxis;

    mGlobalMatrix.r[0].f[3] = 0.0f;
    mGlobalMatrix.r[1].f[3] = 0.0f;
    mGlobalMatrix.r[2].f[3] = 0.0f;

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_GLOBAL);
}

void Entity::SetLocalMatrix(const Math::Matrix& matrix)
{
    if (matrix == mLocalMatrix)
        return;

    mLocalMatrix = matrix;

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_LOCAL);
}

void Entity::SetLocalPosition(const Vector& pos)
{
    mLocalMatrix.r[3] = pos;
    mLocalMatrix.r[3].f[3] = 1.0f;

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_LOCAL);
}

void Entity::SetLocalOrientation(const Matrix& orientation)
{
    // normalize directions
    const Vector zAxis = VectorNormalize3(orientation[2]);
    const Vector xAxis = VectorNormalize3(VectorCross3(orientation[1], orientation[2]));
    const Vector yAxis = VectorNormalize3(VectorCross3(zAxis, xAxis));

    mLocalMatrix.r[0] = xAxis;
    mLocalMatrix.r[1] = yAxis;
    mLocalMatrix.r[2] = zAxis;

    mLocalMatrix.r[0].f[3] = 0.0f;
    mLocalMatrix.r[1].f[3] = 0.0f;
    mLocalMatrix.r[2].f[3] = 0.0f;

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_LOCAL);
}

void Entity::MarkAsDirty(int flags)
{
    if ((mFlags & (NFE_ENTITY_FLAG_MOVED_LOCAL | NFE_ENTITY_FLAG_MOVED_GLOBAL)) == 0)
    {
        mScene->GetEntitySystem()->OnMarkEntityAsDirty(this, flags);
    }

    mFlags |= flags;
}

Math::Box Entity::GetBoundingBox(bool includeChildren) const
{
    Math::Box box;
    
    // TODO
    UNUSED(includeChildren);

    return box;
}

void Entity::SyncLocalTransform()
{
    if (mParent)
    {
        const Matrix& parentMatrix = mParent->GetGlobalMatrix();

        // TODO this is ultra slow and unstable numerically !!!
        const Matrix parentMatrixInv = MatrixInverse(parentMatrix);
        mLocalMatrix = mGlobalMatrix * parentMatrixInv;
    }
    else
    {
        mLocalMatrix = mGlobalMatrix;
    }
}

void Entity::SyncGlobalTransform()
{
    if (mParent)
    {
        const Matrix& parentMatrix = mParent->GetGlobalMatrix();
        mGlobalMatrix = mLocalMatrix * parentMatrix;
    }
    else
    {
        mGlobalMatrix = mLocalMatrix;
    }
}

void Entity::Update()
{
    NFE_ASSERT(!HasBeenRemoved(), "Trying to update entity that was marked for removal");
    NFE_ASSERT(!(HasMovedLocally() && HasMovedGlobally()), "Both local and global transform are dirty");

    if (HasMovedLocally())
    {
        SyncGlobalTransform();
    }
    else if (HasMovedGlobally())
    {
        SyncLocalTransform();
    }

    for (const ComponentPtr& component : mComponents)
    {
        component->OnUpdate();
    }

    mFlags = 0;
}

} // namespace Scene
} // namespace NFE
