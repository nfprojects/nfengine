#include "PCH.hpp"
#include "TriggeredLightController.hpp"
#include "Main.hpp"

#include "nfCore/Scene/Entity.hpp"
#include "nfCore/Scene/Events/Event_Trigger.hpp"
#include "nfCore/Scene/Components/ComponentLight.hpp"

#include "nfCommon/System/Assertion.hpp"


namespace NFE {

using namespace Renderer;
using namespace Math;
using namespace Scene;
using namespace Resource;


TriggeredLightComponent::TriggeredLightComponent()
{ }

void TriggeredLightComponent::OnEvent(const Scene::Event& event)
{
    if (event.GetDynamicType() == RTTI::GetType<Scene::Event_Trigger>())
    {
        const Scene::Event_Trigger& triggerEvent = static_cast<const Scene::Event_Trigger&>(event);
    }
}


} // namespace NFE
