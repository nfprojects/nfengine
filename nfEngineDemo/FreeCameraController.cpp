#include "PCH.hpp"
#include "FreeCameraController.hpp"
#include "Test.hpp"

#include "nfCore/Scene/GameObjectInstance.hpp"

using namespace NFE;
using namespace NFE::Renderer;
using namespace NFE::Math;
using namespace NFE::Scene;
using namespace NFE::Resource;

#define CAMERA_ROTATION_SMOOTHING 0.05f
#define CAMERA_TRANSLATION_SMOOTHING 0.2f

FreeCameraController::FreeCameraController(GameObjectInstance* instance)
    : GameObjectController(instance)
    , mCameraYaw(0.0f)
    , mCameraPitch(0.0f)
{
    mOrientation = QuaternionMultiply(QuaternionRotationY(mCameraYaw), QuaternionRotationX(-mCameraPitch));
}

void FreeCameraController::OnTick(float deltaTime)
{
    GameObjectInstance* instance = GetInstance();
    NFE_ASSERT(instance, "Not attached to game object instance");

    Entity* entity = instance->GetEntity();

    BodyComponent* body = entity->GetComponent<BodyComponent>();
    NFE_ASSERT(body, "Body component not found");

    Quaternion destOrientation = QuaternionMultiply(QuaternionRotationY(mCameraYaw),
                                                    QuaternionRotationX(-mCameraPitch));
    destOrientation = QuaternionNormalize(destOrientation);

    // LPF
    Quaternion prevOrientation = mOrientation;
    mOrientation = QuaternionInterpolate(mOrientation, destOrientation,
                                         deltaTime / (CAMERA_ROTATION_SMOOTHING + deltaTime));
    mOrientation = QuaternionNormalize(mOrientation);

    Quaternion rotation = QuaternionMultiply(prevOrientation, QuaternionInverse(mOrientation));
    body->SetAngularVelocity(-QuaternionToAxis(rotation) / deltaTime);

    Matrix rotMatrix = MatrixFromQuaternion(QuaternionNormalize(mOrientation));
    entity->SetGlobalOrientation(rotMatrix);


    Vector destVelocity = Vector();
    if (cameraControl)
    {
        if (IsKeyPressed(Common::KeyCode::W)) destVelocity += orient.z;
        if (IsKeyPressed(Common::KeyCode::S)) destVelocity -= orient.z;
        if (IsKeyPressed(Common::KeyCode::D)) destVelocity += orient.x;
        if (IsKeyPressed(Common::KeyCode::A)) destVelocity -= orient.x;
        if (IsKeyPressed(Common::KeyCode::R)) destVelocity += orient.y;
        if (IsKeyPressed(Common::KeyCode::F)) destVelocity -= orient.y;

        if (IsKeyPressed(Common::KeyCode::ShiftLeft)) destVelocity *= 30.0f;
        else if (IsKeyPressed(Common::KeyCode::ControlLeft)) destVelocity *= 0.2f;
        else destVelocity *= 3.0f;
    }

    Vector prevVelocity = body->GetVelocity();

    // low pass filter - for smooth camera movement
    float factor = deltaTime / (CAMERA_TRANSLATION_SMOOTHING + deltaTime);
    body->SetVelocity(VectorLerp(prevVelocity, destVelocity, factor));
}

void FreeCameraController::OnKeyPressed(Common::KeyCode key)
{

}

void FreeCameraController::OnKeyReleased(Common::KeyCode key)
{

}