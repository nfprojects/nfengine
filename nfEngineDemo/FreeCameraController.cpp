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
    mOrientation = QuaternionMultiply(QuaternionRotationY(mCameraYaw), QuaternionRotationX(-mCameraPitch));
}

void FreeCameraController::OnTickEvent(const Event& event)
{
    const EventData_Tick* tickEvent = static_cast<const EventData_Tick*>(event.data);

    Entity* entity = GetEntity();
    NFE_ASSERT(entity, "Invalid entity");

    BodyComponent* body = entity->GetComponent<BodyComponent>();
    NFE_ASSERT(body, "Body component not found");

    // calculate desired orientation
    Quaternion destOrientation = QuaternionFromAngles(mCameraPitch, mCameraYaw, 0.0f);
    destOrientation = QuaternionNormalize(destOrientation);

    // low pass filter - for smooth camera rotation
    Quaternion prevOrientation = mOrientation;
    mOrientation = QuaternionInterpolate(mOrientation, destOrientation,
                                         tickEvent->deltaTime / (CAMERA_ROTATION_SMOOTHING + tickEvent->deltaTime));
    mOrientation = QuaternionNormalize(mOrientation);

    const Quaternion rotation = QuaternionMultiply(prevOrientation, QuaternionInverse(mOrientation));
    body->SetAngularVelocity(-QuaternionToAxis(rotation) / tickEvent->deltaTime);

    const Matrix rotMatrix = QuaternionToMatrix(QuaternionNormalize(mOrientation));
    entity->SetGlobalOrientation(rotMatrix);


    const Vector destVelocity = QuaternionRotateVector(rotation, mMovementDirection);
    const Vector prevVelocity = body->GetVelocity();

    // low pass filter - for smooth camera movement
    float factor = tickEvent->deltaTime / (CAMERA_TRANSLATION_SMOOTHING + tickEvent->deltaTime);
    body->SetVelocity(VectorLerp(prevVelocity, destVelocity, factor));
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
