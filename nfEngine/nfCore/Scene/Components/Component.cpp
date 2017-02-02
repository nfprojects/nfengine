/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base definition.
 */

#include "PCH.hpp"
#include "Component.hpp"
#include "../Entity.hpp"

#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/System/Assertion.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::IComponent)
NFE_END_DEFINE_CLASS()


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

SceneManager& IComponent::GetScene() const
{
    NFE_ASSERT(mEntity, "Component is not attached to an entity");
    return mEntity->GetScene();
}

Math::Box IComponent::GetBoundingBox() const
{
    return Math::Box::Empty();
}

void IComponent::Attach(Entity& entity)
{
    NFE_ASSERT(mEntity == nullptr, "Component is already attached to an entity");

    mEntity = &entity;
    OnAttach();
}

void IComponent::Detach(Entity& entity)
{
    NFE_ASSERT(mEntity != nullptr, "Component is not attached to any entity");
    NFE_ASSERT(mEntity == &entity, "Invalid entity pointer provided");

    OnDetach();
    mEntity = nullptr;
}

void IComponent::OnAttach()
{
    // nothing to do by default
}

void IComponent::OnDetach()
{
    // nothing to do by default
}

void IComponent::OnUpdate()
{
    // nothing to do by default
}

} // namespace Scene
} // namespace NFE
