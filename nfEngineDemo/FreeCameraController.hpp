#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/GameObjectController.hpp"

NFE_ALIGN16
class FreeCameraController
    : public NFE::Scene::GameObjectController
    , public NFE::Common::Aligned<16>
{
public:
    FreeCameraController(NFE::Scene::GameObjectInstance* instance);

    void OnTick(float deltaTime);

    // TODO
    void OnKeyPressed(NFE::Common::KeyCode key);
    void OnKeyReleased(NFE::Common::KeyCode key);

private:
    NFE::Math::Quaternion mOrientation;
    float mCameraYaw;
    float mCameraPitch;
};