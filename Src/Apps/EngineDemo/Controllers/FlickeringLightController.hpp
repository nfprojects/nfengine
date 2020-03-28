#pragma once

#include "Engine/Core/Scene/Scene.hpp"
#include "Engine/Core/Scene/EntityController.hpp"
#include "Engine/Core/Scene/Events/Event.hpp"

#include "Engine/Common/Math/Random.hpp"

namespace NFE {

/**
 * Example entity controller implementation - light that flickers randomly.
 */
class FlickeringLightController : public Scene::IEntityController
{
public:
    FlickeringLightController(
        const Math::Vec3f& baseColor = Math::Vec3f(1.0f, 1.0f, 1.0f),
        float frequency = 0.02f);

    virtual void OnEvent(const Scene::Event& event) override;

private:
    Math::Vec3f mBaseColor;

    // 0.0f - no flicker, 1.0f - no light all the time
    float mFrequency;
};

} // namespace NFE
