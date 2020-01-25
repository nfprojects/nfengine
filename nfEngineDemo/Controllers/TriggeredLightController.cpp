#include "PCH.hpp"
#include "TriggeredLightController.hpp"
#include "Main.hpp"

#include "nfCore/Scene/Scene.hpp"
#include "nfCore/Scene/Entity.hpp"
#include "nfCore/Scene/Events/Event_Trigger.hpp"
#include "nfCore/Scene/Components/ComponentLight.hpp"

#include "nfCommon/System/Assertion.hpp"
#include "nfCommon/Reflection/ReflectionTypeResolver.hpp"

namespace NFE {

using namespace Math;
using namespace Resource;


TriggeredLightController::TriggeredLightController()
    : mTriggerCounter(0)
{
}

TriggeredLightController::~TriggeredLightController()
{
}

void TriggeredLightController::OnEvent(const Scene::Event& event)
{
    if (event.GetDynamicType() == RTTI::GetType<Scene::Event_Trigger>())
    {
        const auto& triggerEvent = static_cast<const Scene::Event_Trigger&>(event);

        const Scene::Entity* entity = GetEntity();
        NFE_ASSERT(entity, "Invalid entity");

        Scene::LightComponent* light = entity->GetComponent<Scene::LightComponent>();
        NFE_ASSERT(light, "Light component not found");

        if (triggerEvent.GetType() == Scene::Event_Trigger::Type::Enter)
        {
            if (mTriggerCounter == 0)
            {
                light->SetColor(Float3(20.0f, 20.0f, 20.0f));
            }

            mTriggerCounter++;
        }
        else
        {
            NFE_ASSERT(mTriggerCounter > 0, "Invalid trigger counter");
            mTriggerCounter--;

            if (mTriggerCounter == 0)
            {
                light->SetColor(Float3(0.0f, 0.0f, 0.0f));
            }
        }
    }
}

} // namespace NFE
