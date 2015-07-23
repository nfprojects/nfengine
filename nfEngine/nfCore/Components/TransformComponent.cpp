/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of transform component.
 */

#include "../PCH.hpp"
#include "TransformComponent.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

TransformComponent::TransformComponent()
{
    mFlags = 0;
}

void TransformComponent::SetMatrix(const Matrix& matrix)
{
    mFlags |= NFE_TRANSFORM_FLAG_GLOBAL_MOVED;
    mMatrix = matrix;
}

void TransformComponent::SetPosition(const Vector& pos)
{
    mFlags |= NFE_TRANSFORM_FLAG_GLOBAL_MOVED;

    mMatrix.r[3] = pos;
    mMatrix.r[3].m128_f32[3] = 1.0f;
}

void TransformComponent::SetOrientation(const Quaternion& quat)
{
    mFlags |= NFE_TRANSFORM_FLAG_GLOBAL_MOVED;

    Matrix rotMatrix = MatrixFromQuaternion(QuaternionNormalize(quat));
    mMatrix.r[0] = rotMatrix.r[0];
    mMatrix.r[1] = rotMatrix.r[1];
    mMatrix.r[2] = rotMatrix.r[2];
}

void TransformComponent::SetOrientation(const Orientation* pOrientation)
{
    mFlags |= NFE_TRANSFORM_FLAG_GLOBAL_MOVED;

    //normalize directions
    Vector zAxis, yAxis, xAxis;
    zAxis = VectorNormalize3(pOrientation->z);
    xAxis = VectorNormalize3(VectorCross3(pOrientation->y, pOrientation->z));
    yAxis = VectorNormalize3(VectorCross3(zAxis, xAxis));

    mMatrix.r[0] = xAxis;
    mMatrix.r[1] = yAxis;
    mMatrix.r[2] = zAxis;

    mMatrix.r[0].m128_f32[3] = 0.0f;
    mMatrix.r[1].m128_f32[3] = 0.0f;
    mMatrix.r[2].m128_f32[3] = 0.0f;
}

void TransformComponent::GetOrientation(Orientation* orientation) const
{
    orientation->x = mMatrix.r[0];
    orientation->y = mMatrix.r[1];
    orientation->z = mMatrix.r[2];
}

void TransformComponent::SetLocalPosition(const Vector& pos)
{
    mLocalMatrix.r[3] = pos;
    mLocalMatrix.r[3].m128_f32[3] = 1.0f;

    mFlags |= NFE_TRANSFORM_FLAG_LOCAL_MOVED;
}

Vector TransformComponent::GetLocalPosition() const
{
    return mLocalMatrix.r[3];
}

void TransformComponent::SetLocalOrientation(const Orientation* pOrientation)
{
    //normalize directions
    Vector zAxis, yAxis, xAxis;
    zAxis = VectorNormalize3(pOrientation->z);
    xAxis = VectorNormalize3(VectorCross3(pOrientation->y, pOrientation->z));
    yAxis = VectorNormalize3(VectorCross3(zAxis, xAxis));

    mLocalMatrix.r[0] = xAxis;
    mLocalMatrix.r[1] = yAxis;
    mLocalMatrix.r[2] = zAxis;

    mLocalMatrix.r[0].m128_f32[3] = 0.0f;
    mLocalMatrix.r[1].m128_f32[3] = 0.0f;
    mLocalMatrix.r[2].m128_f32[3] = 0.0f;

    mFlags |= NFE_TRANSFORM_FLAG_LOCAL_MOVED;
}

void TransformComponent::GetLocalOrientation(Orientation* pOrientation) const
{
    pOrientation->x = mLocalMatrix.r[0];
    pOrientation->y = mLocalMatrix.r[1];
    pOrientation->z = mLocalMatrix.r[2];
}

} // namespace Scene
} // namespace NFE
