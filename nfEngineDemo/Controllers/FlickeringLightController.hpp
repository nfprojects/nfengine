#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/EntityController.hpp"
#include "nfCore/Scene/Events/Event.hpp"

#include "nfCommon/Math/Random.hpp"

namespace NFE {

/**
 * Example entity controller implementation - light that flickers randomly.
 */
class FlickeringLightController : public Scene::IEntityController
{
public:
    FlickeringLightController(
        const Math::Float3& baseColor = Math::Float3(1.0f, 1.0f, 1.0f),
        float frequency = 0.02f);

    virtual void OnEvent(const Scene::Event& event) override;

private:
    Math::Random mRandomGenerator;
    Math::Float3 mBaseColor;

    // 0.0f - no flicker, 1.0f - no light all the time
    float mFrequency;
};

} // namespace NFE
