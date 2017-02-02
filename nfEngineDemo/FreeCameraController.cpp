#include "PCH.hpp"
#include "FreeCameraController.hpp"
#include "Main.hpp"

#include "nfCore/Scene/Entity.hpp"
#include "nfCore/Scene/Events/Event_Input.hpp"
#include "nfCore/Scene/Events/Event_Tick.hpp"
#include "nfCore/Scene/Components/ComponentBody.hpp"

#include "nfCommon/System/Assertion.hpp"


namespace NFE {

using namespace Renderer;
using namespace Math;
using namespace Scene;
using namespace Resource;

namespace {

const float CAMERA_ROTATION_SMOOTHING = 0.02f;
const float CAMERA_TRANSLATION_SMOOTHING = 0.2f;

} // namespace


FreeCameraController::FreeCameraController()
    : mCameraYaw(0.0f)
    , mCameraPitch(0.0f)
    , mInControl(false)
{
}

void FreeCameraController::OnEvent(const Scene::Event& event)
{
    if (event.GetDynamicType() == RTTI::GetType<Scene::Event_Input>())
    {
        const Scene::Event_Input& inputEvent = static_cast<const Scene::Event_Input&>(event);
        ProcessInputEvent(inputEvent);
    }
    else if (event.GetDynamicType() == RTTI::GetType<Scene::Event_Tick>())
    {
        const Scene::Event_Tick& tickEvent = static_cast<const Scene::Event_Tick&>(event);
        ProcessTickEvent(tickEvent);
    }
}

void FreeCameraController::ProcessTickEvent(const Event_Tick& tickEvent)
{
    Entity* entity = GetEntity();
    NFE_ASSERT(entity, "Invalid entity");

    BodyComponent* body = entity->GetComponent<BodyComponent>();
    NFE_ASSERT(body, "Body component not found");

    SceneManager* scene = entity->GetScene();
    NFE_ASSERT(scene, "Invalid scene");

    InputSystem* inputSystem = scene->GetInputSystem();
    NFE_ASSERT(inputSystem, "Invalid input system");

    // time delta
    const float dt = tickEvent.GetTimeDelta();

    // clamp pitch to -90 ... +90 degrees
    if (mCameraPitch > NFE_MATH_PI / 2.0f)  mCameraPitch = NFE_MATH_PI / 2.0f;
    if (mCameraPitch < -NFE_MATH_PI / 2.0f) mCameraPitch = -NFE_MATH_PI / 2.0f;

    // clamp yaw to -180 ... +180 degrees
    if (mCameraYaw > NFE_MATH_PI)   mCameraYaw -= NFE_MATH_2PI;
    if (mCameraYaw < -NFE_MATH_PI)  mCameraYaw += NFE_MATH_2PI;

    // calculate desired orientation
    const Quaternion targetOrientation = Quaternion::FromAngles(mCameraPitch, mCameraYaw, 0.0f);

    // low pass filter - for smooth camera rotation
    const Quaternion prevOrientation = entity->GetGlobalRotation();

    const Quaternion rotation = prevOrientation * targetOrientation.Inverted();
    {
        Vector axis;
        float angle;
        rotation.ToAxis(axis, angle);
        body->SetAngularVelocity(-axis / dt);
    }

    entity->SetGlobalOrientation(targetOrientation);

    Vector movementDirection = mMovementDirection;
    if (!Vector::AlmostEqual(movementDirection, Vector()))
    {
        movementDirection.Normalize3();
    }

    const Vector destVelocity = targetOrientation.Transform(movementDirection);
    const Vector prevVelocity = destVelocity; // TODO body->GetVelocity();

    // low pass filter - for smooth camera movement
    const float translationFactor = dt / (CAMERA_TRANSLATION_SMOOTHING + dt);
    const Vector newVelocity = Vector::Lerp(prevVelocity, destVelocity, translationFactor);

    const Vector newPosition = entity->GetGlobalPosition() + newVelocity * dt;
    entity->SetGlobalPosition(newPosition);

    // TODO
    // body->SetVelocity(newVelocity);
}

void FreeCameraController::ProcessInputEvent(const Scene::Event_Input& inputEvent)
{
    using namespace Input;
    const EventData& data = inputEvent.GetData();

    if (data.type == EventData::Type::Axis)
    {
        if (data.name == "MoveRightLeft")
        {
            mMovementDirection[0] = data.axisValue;
        }
        else if (data.name == "MoveForwardBackward")
        {
            mMovementDirection[2] = data.axisValue;
        }
        else if (data.name == "MoveUpDown")
        {
            mMovementDirection[1] = data.axisValue;
        }

        if (mInControl)
        {
            if (data.name == "CameraPitch")
            {
                mCameraPitch += data.axisValue;
            }
            else if (data.name == "CameraYaw")
            {
                mCameraYaw += data.axisValue;
            }
        }
    }
    else if (data.type == EventData::Type::KeyPress)
    {
        if (data.name == "CameraControl")
        {
            mInControl = true;
        }
    }
    else if (data.type == EventData::Type::KeyRelease)
    {
        if (data.name == "CameraControl")
        {
            mInControl = false;
        }
    }
}

} // namespace NFE
