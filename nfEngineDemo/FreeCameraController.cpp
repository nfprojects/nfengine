#include "PCH.hpp"
#include "FreeCameraController.hpp"
#include "Main.hpp"

#include "nfCore/Scene/Entity.hpp"
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
    mOrientation = Quaternion::FromAngles(mCameraPitch, mCameraYaw, 0.0f);
}

void FreeCameraController::OnTickEvent(const Event& event)
{
    const EventData_Tick* tickEvent = static_cast<const EventData_Tick*>(event.data);

    Entity* entity = GetEntity();
    NFE_ASSERT(entity, "Invalid entity");

    BodyComponent* body = entity->GetComponent<BodyComponent>();
    NFE_ASSERT(body, "Body component not found");

    // calculate desired orientation
    Quaternion destOrientation = Quaternion::FromAngles(mCameraPitch, mCameraYaw, 0.0f);

    // low pass filter - for smooth camera rotation
    Quaternion prevOrientation = mOrientation;
    const float rotationFactor = tickEvent->deltaTime / (CAMERA_ROTATION_SMOOTHING + tickEvent->deltaTime);
    mOrientation = Quaternion::Interpolate(mOrientation, destOrientation, rotationFactor).Normalized();

    const Quaternion rotation = prevOrientation * mOrientation.Inverted();

    {
        Vector axis;
        float angle;
        rotation.ToAxis(axis, angle);
        body->SetAngularVelocity(-axis / tickEvent->deltaTime);
    }

    entity->SetGlobalOrientation(mOrientation);


    const Vector destVelocity = rotation.Transform(mMovementDirection);
    const Vector prevVelocity = body->GetVelocity();

    // low pass filter - for smooth camera movement
    const float translationFactor = tickEvent->deltaTime / (CAMERA_TRANSLATION_SMOOTHING + tickEvent->deltaTime);
    body->SetVelocity(Vector::Lerp(prevVelocity, destVelocity, translationFactor));
}

void FreeCameraController::OnInputEvent(const Input::Event& event)
{
    if (event.type == Input::Event::Type::Axis)
    {
        if (strcmp(event.name, "MoveLeftRight") == 0)
        {
            mMovementDirection[0] = event.axisValue;
        }
        else if (strcmp(event.name, "MoveForwardBackward") == 0)
        {
            mMovementDirection[2] = event.axisValue;
        }
        else if (strcmp(event.name, "MoveUpDown") == 0)
        {
            mMovementDirection[1] = event.axisValue;
        }
    }
    else if (event.type == Input::Event::Type::KeyPress)
    {

    }
}

} // namespace NFE
