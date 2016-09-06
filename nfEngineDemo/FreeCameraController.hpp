#pragma once

#include "nfCore/Engine.hpp"
#include "nfCore/Scene/EntityController.hpp"

NFE_ALIGN16
class FreeCameraController
    : public NFE::Scene::EntityController
    , public NFE::Common::Aligned<16>
{
public:
    FreeCameraController(NFE::Scene::Entity* entity);

    void OnTick(float deltaTime);

    // TODO
    void OnKeyPressed(Common::KeyCode key);
    void OnKeyReleased(Common::KeyCode key);

private:
    Quaternion mOrientation;
    float mCameraYaw;
    float mCameraPitch;
};