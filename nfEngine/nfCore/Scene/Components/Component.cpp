/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base definition.
 */

#include "PCH.hpp"
#include "Component.hpp"
#include "../Entity.hpp"

#include "nfCommon/Logger.hpp"
#include "nfCommon/Assertion.hpp"


namespace NFE {
namespace Scene {

IComponent::IComponent()
    : mEntity(nullptr)
{
}

IComponent::~IComponent()
{
    NFE_ASSERT(mEntity == nullptr, "Component must be detached before being destroyed");
}

void IComponent::OnAttach(Entity* entity)
{
    UNUSED(entity);
    // nothing by default
}

void IComponent::OnDetach()
{
    // nothing by default
}

SceneManager* IComponent::GetScene() const
{
    return mEntity->GetScene();
}

} // namespace Scene
} // namespace NFE
