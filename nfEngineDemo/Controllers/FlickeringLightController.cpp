#include "PCH.hpp"
#include "FlickeringLightController.hpp"

#include "nfCore/Scene/Entity.hpp"
#include "nfCore/Scene/Events/Event_Tick.hpp"
#include "nfCore/Scene/Components/ComponentLight.hpp"

#include "nfCommon/System/Assertion.hpp"

namespace NFE {

using namespace Scene;
using namespace Math;

FlickeringLightController::FlickeringLightController(const Float3& baseColor, float freqency)
    : mRandomGenerator((uint64)this)
    , mBaseColor(baseColor)
    , mFrequency(freqency)
{ }

void FlickeringLightController::OnEvent(const Scene::Event& event)
{
    if (event.GetDynamicType() == RTTI::GetType<Scene::Event_Tick>())
    {
        const Entity* entity = GetEntity();
        NFE_ASSERT(entity, "Invalid entity");

        LightComponent* light = entity->GetComponent<LightComponent>();
        NFE_ASSERT(light, "Light component not found");

        const float lightValue = mRandomGenerator.GetFloat() < mFrequency ? 0.5f : 1.0f;
        light->SetColor(mBaseColor * lightValue);
    }
}

} // namespace NFE
