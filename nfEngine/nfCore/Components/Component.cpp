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
    : mEntity(nullptr)
    , mComponentType(id)
{
}

Component::~Component()
{
    NFE_ASSERT(mEntity == nullptr, "Component must be detached before being destroyed");
}

void Component::OnAttachToEntity(Entity* entity)
{
    mEntity = entity;
}

void Component::OnChanged()
{
    // TODO
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
