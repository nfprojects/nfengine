/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of trigger component class.
 */

#include "PCH.hpp"
#include "ComponentTrigger.hpp"
#include "Engine.hpp"
#include "../Entity.hpp"
#include "../SceneManager.hpp"
#include "../Systems/TriggerSystem.hpp"

#include "nfCommon/System/Assertion.hpp"


namespace NFE {
namespace Scene {

using namespace Math;

TriggerComponent::TriggerComponent()
{
}

TriggerComponent::~TriggerComponent()
{
}

Math::Box TriggerComponent::GetBoundingBox() const
{
    Entity* entity = GetEntity();
    NFE_ASSERT(entity, "Component is not attached to entity");

    const Vector& center = entity->GetGlobalPosition();
    Box triggerBox(center - mSize, center + mSize);
    return triggerBox;
}

TriggerSystem* TriggerComponent::GetTriggerSystem() const
{
    Entity* entity = GetEntity();
    if (!GetEntity())
        return nullptr;

    SceneManager* scene = entity->GetScene();
    NFE_ASSERT(scene, "Entity not attached to a scene");

    TriggerSystem* triggerSystem = scene->GetTriggerSystem();
    NFE_ASSERT(triggerSystem, "Trigger system is not present");

    return triggerSystem;
}

void TriggerComponent::OnAttach()
{
    TriggerSystem* system = GetTriggerSystem();
    if (!system)
        return;

    TriggerDesc desc;
    desc.box = GetBoundingBox();
    // TODO filters

    mID = system->CreateTrigger(desc);
}

void TriggerComponent::OnDetach()
{
    TriggerSystem* system = GetTriggerSystem();
    if (!system)
        return;

    system->DestroyTrigger(mID);
}

void TriggerComponent::SetSize(const Math::Vector& newSize)
{

}

} // namespace Scene
} // namespace NFE
