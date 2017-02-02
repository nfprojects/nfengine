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
    // TODO this is temporary - RTTI will be used here
    if (event.GetDynamicType() == NFE_GET_TYPE(Scene::Event_Input))
    {
        const Scene::Event_Input& inputEvent = static_cast<const Scene::Event_Input&>(event);
        ProcessInputEvent(inputEvent);
    }
    else if (event.GetDynamicType() == NFE_GET_TYPE(Scene::Event_Tick))
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


    const Vector destVelocity = rotation.Transform(mMovementDirection);
    const Vector prevVelocity = body->GetVelocity();

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
        if (strcmp(data.name, "MoveLeftRight") == 0)
        {
            mMovementDirection[0] = data.axisValue;
        }
        else if (strcmp(data.name, "MoveForwardBackward") == 0)
        {
            mMovementDirection[2] = data.axisValue;
        }
        else if (strcmp(data.name, "MoveUpDown") == 0)
        {
            mMovementDirection[1] = data.axisValue;
        }

        if (mInControl)
        {
            if (strcmp(data.name, "CameraPitch") == 0)
            {
                mCameraPitch += data.axisValue;
            }
            else if (strcmp(data.name, "CameraYaw") == 0)
            {
                mCameraYaw += data.axisValue;
            }
        }
    }
    else if (data.type == EventData::Type::KeyPress)
    {
        if (strcmp(data.name, "CameraControl") == 0)
        {
            mInControl = true;
        }
    }
    else if (data.type == EventData::Type::KeyRelease)
    {
        if (strcmp(data.name, "CameraControl") == 0)
        {
            mInControl = false;
        }
    }
}

} // namespace NFE
