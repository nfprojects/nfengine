/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of transform system.
 */

#include "PCH.hpp"
#include "TransformSystem.hpp"
#include "Engine.hpp"
#include "Scene/EntityManager.hpp"
#include "Components/TransformComponent.hpp"

#include "nfCommon/Logger/Logger.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

TransformSystem::TransformSystem(SceneManager* scene)
{
    mEntityManager = scene->GetEntityManager();
}

void TransformSystem::UpdateComponent(TransformComponent* component)
{
    if (component->mFlags)
    {
        TransformComponent* parent = component->mParent;

        if (component->mFlags & NFE_TRANSFORM_FLAG_GLOBAL_MOVED)
        {
            if (parent)
            {
                Matrix parentMatrixInverse = MatrixInverse(parent->mMatrix);
                component->mLocalMatrix = component->mMatrix * parentMatrixInverse;
            }
            else
                component->mLocalMatrix = component->mMatrix;
        }
        else if (component->mFlags & NFE_TRANSFORM_FLAG_LOCAL_MOVED)
        {
            if (parent)
                component->mMatrix = component->mLocalMatrix * parent->mMatrix;
            else
                component->mMatrix = component->mLocalMatrix;
        }

        for (auto child : component->mChildren)
        {
            child->mFlags |= NFE_TRANSFORM_FLAG_LOCAL_MOVED;
            UpdateComponent(child);
        }

        component->mFlags = 0;
    }
}

void TransformSystem::Update()
{
    mEntityManager->ForEach<TransformComponent>(
        [this](EntityID, TransformComponent* transform)
        {
            UpdateComponent(transform);
        });
}

bool TransformSystem::SetParent(EntityID entity, EntityID newParent)
{
    TransformComponent* transform;
    TransformComponent* parentTransform;

    transform = mEntityManager->GetComponent<TransformComponent>(entity);
    if (!transform)
    {
        LOG_ERROR("Entity %u does not have Transform Component", entity);
        return false;
    }

    if (newParent == gInvalidEntityID)  // detach from parent
    {
        if (transform->mParent)
        {
            transform->mParent->mChildren.erase(transform);
            transform->mParent = nullptr;
        }
    }
    else
    {
        parentTransform = mEntityManager->GetComponent<TransformComponent>(newParent);
        if (!parentTransform)
        {
            LOG_ERROR("Entity %u does not have Transform Component", entity);
            return false;
        }

        // cleanup old parent
        if (transform->mParent != nullptr)
            transform->mParent->mChildren.erase(transform);

        // check if new parent is valid (won't create a loop)
        TransformComponent* parent = parentTransform;
        while (parent)
        {
            if (parent == transform)
            {
                LOG_ERROR("Invalid parent set to entity (child = %u, newParent = %u)",
                          entity, newParent);
                return false;
            }

            parent = parent->mParent;
        }

        transform->mParent = parentTransform;
        parentTransform->mChildren.insert(transform);
    }

    return true;
}

} // namespace Scene
} // namespace NFE
