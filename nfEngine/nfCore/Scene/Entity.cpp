/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Entity class.
 */

#include "PCH.hpp"
#include "Entity.hpp"
#include "EntityController.hpp"
#include "SceneManager.hpp"
#include "Components/Component.hpp"
#include "Systems/EntitySystem.hpp"

#include "nfCommon/System/Assertion.hpp"


namespace NFE {
namespace Scene {

using namespace Math;

Entity::Entity(EntityID id, SceneManager* parentScene, Entity* parentEntity)
    : mParent(parentEntity)
    , mScene(parentScene)
    , mID(id)
    , mFlags(0)
{
    NFE_ASSERT(parentScene, "Invalid entity scene");
}

Entity::~Entity()
{
    NFE_ASSERT(mParent == nullptr, "Destroyed entity has still a parent");

    DetachComponents();
    DetachController();
}

void Entity::Remove(bool removeChildren)
{
    if (HasBeenRemoved())
        return;

    MarkAsDirty(NFE_ENTITY_FLAG_TO_REMOVE);

    // remove child entities
    if (removeChildren)
    {
        for (Entity* child : mChildren)
        {
            child->Remove();
        }
    }

    DetachComponents();
    DetachController();
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

bool Entity::AttachChildEntity(Entity* child)
{
    NFE_ASSERT(child->GetParent() == nullptr, "Cannot attach entity that has a parent");
    NFE_ASSERT(child->GetScene() == GetScene(), "Child entity must belong to the same scene");

    mChildren.push_back(child);
    child->mParent = this;
    child->MarkAsDirty(NFE_ENTITY_FLAG_MOVED_GLOBAL); // so local transform gets updated

    // TODO notify Game Object Instance

    return true;
}

void Entity::AttachController(EntityControllerPtr&& controller)
{
    if (mController)
    {
        mController->Detach();
        mController.reset();
    }

    mController = std::move(controller);
    mController->Attach(this);
}

bool Entity::DetachController()
{
    if (mController)
    {
        mController->Detach();
        mController.reset();
        return true;
    }

    return false;
}

bool Entity::DetachChildEntity(Entity* child)
{
    // TODO optimize
    auto iter = std::find(mChildren.begin(), mChildren.end(), child);
    NFE_ASSERT(iter != mChildren.end(), "Child entity not found");

    child->mParent = nullptr;
    mChildren.erase(iter);

    // TODO notify Game Object Instance

    return true;
}

bool Entity::SetParentEntity(Entity* newParent)
{
    // TODO
    return false;
}


void Entity::SetGlobalTransform(const Transform& transform)
{
    if (Math::Transform::AlmostEqual(transform, mGlobalTransform))
        return;

    mGlobalTransform = transform;

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_GLOBAL);
}

void Entity::SetGlobalPosition(const Vector& pos)
{
    if (mGlobalTransform.GetTranslation() == pos)
        return;

    mGlobalTransform.SetTranslation(pos);

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_GLOBAL);
}

void Entity::SetGlobalOrientation(const Quaternion& quat)
{
    if (Quaternion::AlmostEqual(mGlobalTransform.GetRotations(), quat))
        return;

    mGlobalTransform.SetRotation(quat);

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_GLOBAL);
}


void Entity::SetLocalTransform(const Transform& transform)
{
    if (Math::Transform::AlmostEqual(transform, mLocalTransform))
        return;

    mLocalTransform = transform;

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_LOCAL);
}

void Entity::SetLocalPosition(const Vector& pos)
{
    if (mLocalTransform.GetTranslation() == pos)
        return;

    mLocalTransform.SetTranslation(pos);

    MarkAsDirty(NFE_ENTITY_FLAG_MOVED_LOCAL);
}

void Entity::SetLocalOrientation(const Quaternion& quat)
{
    if (Quaternion::AlmostEqual(mLocalTransform.GetRotations(), quat))
        return;

    mLocalTransform.SetRotation(quat);

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

Box Entity::GetBoundingBox(bool includeChildren) const
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
        const Transform& parentTransform = mParent->GetGlobalTransform();
        mLocalTransform = mGlobalTransform * parentTransform.Inverted();
    }
    else
    {
        mLocalTransform = mGlobalTransform;
    }
}

void Entity::SyncGlobalTransform()
{
    if (mParent)
    {
        const Transform& parentTransform = mParent->GetGlobalTransform();
        mGlobalTransform = mLocalTransform * parentTransform;
    }
    else
    {
        mGlobalTransform = mLocalTransform;
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
