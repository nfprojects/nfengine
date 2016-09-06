/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base definition.
 */

#include "PCH.hpp"
#include "Component.hpp"
#include "../Scene/EntityManager.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Scene {

std::vector<ComponentInfo> Component::mComponentTypes;
int Component::mComponentIdCounter = 0;

Component::Component(int id)
    : mEntityManager(nullptr)
    , mEntity(0)
    , mComponentType(id)
{
}

void Component::OnAttachToEntity(EntityManager* manager, EntityID entity)
{
    mEntityManager = manager;
    mEntity = entity;
}

void Component::HasChanged() const
{
    if (!mEntityManager)
        return;

    mEntityManager->OnComponentChanged(mEntity, mComponentType);
}

void Component::Invalidate()
{
}

bool Component::RegisterType(int id, const char* name, size_t size)
{
    ComponentInfo info;
    info.id = id;
    info.name = name;
    info.size = size;
    mComponentTypes.push_back(info);

    LOG_INFO("Component type '%s' registered: ID = %i, size = %zu", name, id, size);
    return true;
}

} // namespace Scene
} // namespace NFE
