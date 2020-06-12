#include "PCH.hpp"
#include "FlickeringLightController.hpp"

#include "Engine/Core/Scene/Entity.hpp"
#include "Engine/Core/Scene/Events/Event_Tick.hpp"
#include "Engine/Core/Scene/Components/ComponentLight.hpp"

#include "Engine/Common/Reflection/Types/ReflectionClassType.hpp"
#include "Engine/Common/System/Assertion.hpp"

namespace NFE {

using namespace Math;

static Math::Random gRandomGenerator;

FlickeringLightController::FlickeringLightController(const Vec3f& baseColor, float freqency)
    : mBaseColor(baseColor)
    , mFrequency(freqency)
{ }

void FlickeringLightController::OnEvent(const Scene::Event& event)
{
    if (event.GetDynamicType() == RTTI::GetType<Scene::Event_Tick>())
    {
        const Scene::Entity* entity = GetEntity();
        NFE_ASSERT(entity, "Invalid entity");

        Scene::LightComponent* light = entity->GetComponent<Scene::LightComponent>();
        NFE_ASSERT(light, "Light component not found");

        const float lightValue = gRandomGenerator.GetFloat() < mFrequency ? 0.5f : 1.0f;
        light->SetColor(mBaseColor * lightValue);
    }
}

} // namespace NFE
