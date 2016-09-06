/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of transform component.
 */

#include "PCH.hpp"
#include "TransformComponent.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

NFE_REGISTER_COMPONENT(TransformComponent);

TransformComponent::TransformComponent()
    : mFlags(0)
    , mParent(nullptr)
{
}

void TransformComponent::Invalidate()
{
    if (mParent != nullptr)
    {
        mParent->mChildren.erase(this);
        mParent = nullptr;
    }

    for (auto& child : mChildren)
        child->mParent = nullptr;
    mChildren.clear();
}

Orientation TransformComponent::GetOrientation() const
{
    return Orientation(mMatrix);
}

Orientation TransformComponent::GetLocalOrientation() const
{
    return Orientation(mLocalMatrix);
}

void TransformComponent::SetMatrix(const Matrix& matrix)
{
    mFlags |= NFE_TRANSFORM_FLAG_GLOBAL_MOVED;
    mMatrix = matrix;

    HasChanged();
}

void TransformComponent::SetPosition(const Vector& pos)
{
    mFlags |= NFE_TRANSFORM_FLAG_GLOBAL_MOVED;
    mMatrix.r[3] = pos;
    mMatrix.r[3].f[3] = 1.0f;

    HasChanged();
}

void TransformComponent::SetOrientation(const Quaternion& quat)
{
    mFlags |= NFE_TRANSFORM_FLAG_GLOBAL_MOVED;

    Matrix rotMatrix = MatrixFromQuaternion(QuaternionNormalize(quat));
    mMatrix.r[0] = rotMatrix.r[0];
    mMatrix.r[1] = rotMatrix.r[1];
    mMatrix.r[2] = rotMatrix.r[2];

    HasChanged();
}

void TransformComponent::SetOrientation(const Orientation& orientation)
{
    mFlags |= NFE_TRANSFORM_FLAG_GLOBAL_MOVED;

    /// normalize directions
    Vector zAxis, yAxis, xAxis;
    zAxis = VectorNormalize3(orientation.z);
    xAxis = VectorNormalize3(VectorCross3(orientation.y, orientation.z));
    yAxis = VectorNormalize3(VectorCross3(zAxis, xAxis));

    mMatrix.r[0] = xAxis;
    mMatrix.r[1] = yAxis;
    mMatrix.r[2] = zAxis;

    mMatrix.r[0].f[3] = 0.0f;
    mMatrix.r[1].f[3] = 0.0f;
    mMatrix.r[2].f[3] = 0.0f;

    HasChanged();
}

void TransformComponent::SetLocalPosition(const Vector& pos)
{
    mLocalMatrix.r[3] = pos;
    mLocalMatrix.r[3].f[3] = 1.0f;

    mFlags |= NFE_TRANSFORM_FLAG_LOCAL_MOVED;

    HasChanged();
}

void TransformComponent::SetLocalOrientation(const Orientation& orientation)
{
    /// normalize directions
    Vector zAxis, yAxis, xAxis;
    zAxis = VectorNormalize3(orientation.z);
    xAxis = VectorNormalize3(VectorCross3(orientation.y, orientation.z));
    yAxis = VectorNormalize3(VectorCross3(zAxis, xAxis));

    mLocalMatrix.r[0] = xAxis;
    mLocalMatrix.r[1] = yAxis;
    mLocalMatrix.r[2] = zAxis;

    mLocalMatrix.r[0].f[3] = 0.0f;
    mLocalMatrix.r[1].f[3] = 0.0f;
    mLocalMatrix.r[2].f[3] = 0.0f;

    mFlags |= NFE_TRANSFORM_FLAG_LOCAL_MOVED;

    HasChanged();
}

} // namespace Scene
} // namespace NFE
