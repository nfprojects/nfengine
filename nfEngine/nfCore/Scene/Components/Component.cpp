/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base definition.
 */

#include "PCH.hpp"
#include "Component.hpp"
#include "../Scene/Entity.hpp"
#include "nfCommon/Logger.hpp"


namespace NFE {
namespace Scene {

Component::Component()
    : mEntity(nullptr)
{
}

Component::~Component()
{
    NFE_ASSERT(mEntity == nullptr, "Component must be detached before being destroyed");
}

void Component::OnAttach(Entity* entity)
{
    UNUSED(entity);
    // nothing by default
}

void Component::OnDetach()
{
    // nothing by default
}

SceneManager* Component::GetScene() const
{
    return mEntity->GetScene();
}

} // namespace Scene
} // namespace NFE
