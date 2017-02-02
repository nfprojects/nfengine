#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/GameObjectController.hpp"
#include "nfCore/Scene/Events/Event.hpp"
#include "nfCore/Input/InputEvent.hpp"

#include "nfCommon/Math/Quaternion.hpp"


/**
 * Example Game Object Controller implementation - free camera.
 */
NFE_ALIGN16
class FreeCameraController
    : public NFE::Scene::GameObjectController
    , public NFE::Common::Aligned<16>
{
public:
    FreeCameraController(NFE::Scene::GameObjectInstance* instance);

    void OnTickEvent(const NFE::Scene::Event& event);

    void OnInputEvent(const NFE::Input::Event& event);

private:
    float mMovementSpeed;
    NFE::Math::Vector mMovementDirection;
    NFE::Math::Quaternion mOrientation;

    // yaw and pitch angles
    float mCameraYaw;
    float mCameraPitch;

    // is mouse controlling the camera?
    bool mInControl;
};