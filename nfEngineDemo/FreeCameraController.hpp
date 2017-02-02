#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/GameObjectController.hpp"
#include "nfCore/Scene/Events/Event.hpp"
#include "nfCore/Input/InputEvent.hpp"

#include "nfCommon/Math/Quaternion.hpp"

namespace NFE {


/**
 * Example Game Object Controller implementation - free camera.
 */
NFE_ALIGN16
class FreeCameraController
    : public Scene::GameObjectController
    , public Common::Aligned<16>
{
public:
    FreeCameraController(Scene::GameObjectInstance* instance);

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
