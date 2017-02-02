#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/EntityController.hpp"
#include "nfCore/Scene/Events/Event.hpp"
#include "nfCore/Input/InputEvent.hpp"

#include "nfCommon/Math/Quaternion.hpp"

namespace NFE {


/**
 * Example entity controller implementation - free camera.
 */
NFE_ALIGN16
class FreeCameraController
    : public Scene::IEntityController
    , public Common::Aligned<16>
{
public:
    FreeCameraController();

    virtual void OnEvent(const Scene::Event& event) override;

private:
    float mMovementSpeed;
    Math::Vector mMovementDirection;

    // yaw and pitch angles
    float mCameraYaw;
    float mCameraPitch;

    // is mouse controlling the camera?
    bool mInControl;

    void ProcessTickEvent(const Scene::Event_Tick& tickEvent);
    void ProcessInputEvent(const Scene::Event_Input& inputEvent);
};

} // namespace NFE
