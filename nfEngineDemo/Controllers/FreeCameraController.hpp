#pragma once

#include "nfCore/Scene/Scene.hpp"
#include "nfCore/Scene/EntityController.hpp"
#include "nfCore/Scene/Events/Event.hpp"
#include "nfCore/Input/InputEvent.hpp"

#include "nfCommon/Math/Quaternion.hpp"
#include "nfCommon/Memory/Aligned.hpp"

namespace NFE {


/**
 * Example entity controller implementation - free camera.
 */
class NFE_ALIGN(16) FreeCameraController
    : public Scene::IEntityController
    , public Common::Aligned<16>
{
public:
    FreeCameraController();

    virtual void OnEvent(const Scene::Event& event) override;

private:
    Math::Vector4 mMovementDirection;
    float mMovementSpeed;

    // yaw and pitch angles
    float mCameraYaw;
    float mCameraPitch;

    // is mouse controlling the camera?
    bool mInControl;

    void ProcessTickEvent(const Scene::Event_Tick& tickEvent);
    void ProcessInputEvent(const Scene::Event_Input& inputEvent);
};

} // namespace NFE
