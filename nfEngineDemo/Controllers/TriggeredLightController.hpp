#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/EntityController.hpp"
#include "nfCore/Scene/Events/Event.hpp"


namespace NFE {

/**
 * Example entity controller implementation - light that changes a color when camera is inside trigger.
 */
class TriggeredLightController : public Scene::IEntityController
{
public:
    TriggeredLightController();
    ~TriggeredLightController();

    virtual void OnEvent(const Scene::Event& event) override;

private:
    // number of source triggers that entered controlled light
    uint32 mTriggerCounter;
};

} // namespace NFE
