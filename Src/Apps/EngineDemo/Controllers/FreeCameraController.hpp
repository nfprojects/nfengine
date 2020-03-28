#pragma once

#include "Engine/Core/Scene/Scene.hpp"
#include "Engine/Core/Scene/EntityController.hpp"
#include "Engine/Core/Scene/Events/Event.hpp"
#include "Engine/Core/Input/InputEvent.hpp"

#include "Engine/Common/Math/Quaternion.hpp"
#include "Engine/Common/Memory/Aligned.hpp"

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
    Math::Vec4f mMovementDirection;
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
