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


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::TriggerComponent)
    NFE_CLASS_PARENT(NFE::Scene::IComponent)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

using namespace Math;

TriggerComponent::TriggerComponent()
    : mID(InvalidTriggerID)
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
    if (!GetEntity())
        return nullptr;

    TriggerSystem* triggerSystem = GetScene().GetSystem<TriggerSystem>();
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
    desc.entity = GetEntity();
    desc.isTarget = mType == TriggerType::Target;

    mID = system->CreateTrigger(desc);
}

void TriggerComponent::OnDetach()
{
    if (mID == InvalidTriggerID)
        return;

    TriggerSystem* system = GetTriggerSystem();
    if (!system)
        return;

    system->DestroyTrigger(mID);
}

void TriggerComponent::OnUpdate()
{
    if (mID == InvalidTriggerID)
        return;

    TriggerSystem* system = GetTriggerSystem();
    if (!system)
        return;

    system->UpdateTrigger(mID, GetBoundingBox());
}

void TriggerComponent::SetSize(const Math::Vector& newSize)
{
    if (mSize == newSize)
    {
        // nothing to do
        return;
    }

    // TODO update proxy

    mSize = newSize;
}

void TriggerComponent::SetType(TriggerType newType)
{
    if (mType == newType)
    {
        // nothing to do
        return;
    }

    // TODO destroy & create proxy

    mType = newType;
}

} // namespace Scene
} // namespace NFE
