#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/EntityController.hpp"
#include "nfCore/Scene/Events/Event.hpp"


namespace NFE {

/**
 * Example entity controller implementation - light that changes a color when camera is inside trigger.
 */
class NFE_ALIGN16 TriggeredLightComponent
    : public Scene::IEntityController
    , public Common::Aligned<16>
{
public:
    TriggeredLightComponent();

    virtual void OnEvent(const Scene::Event& event) override;
};

} // namespace NFE
