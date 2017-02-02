#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/Components/ComponentScript.hpp"
#include "nfCore/Scene/Events/Event.hpp"
#include "nfCore/Input/InputEvent.hpp"

#include "nfCommon/Math/Quaternion.hpp"

namespace NFE {


/**
 * Example script implementation - free camera.
 */
NFE_ALIGN16
class FreeCameraController
    : public Scene::NativeScript
    , public Common::Aligned<16>
{
public:
    FreeCameraController();

    void OnTickEvent(const Scene::Event& event);
    void OnInputEvent(const Input::Event& event);

private:
    float mMovementSpeed;
    Math::Vector mMovementDirection;
    Math::Quaternion mOrientation;

    // yaw and pitch angles
    float mCameraYaw;
    float mCameraPitch;

    // is mouse controlling the camera?
    bool mInControl;
};

} // namespace NFE
