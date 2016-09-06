/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of Entity class.
 */

#include "PCH.hpp"
#include "Entity.hpp"


namespace NFE {
namespace Scene {

using namespace Math;

Entity::Entity(Entity* parentEntity, SceneManager* parentScene)
    : mParent(parentEntity)
    , mScene(parentScene)
    , mFlags(0)
{}

Entity::~Entity()
{}

Matrix Entity::GetGlobalOrientation() const
{
    Matrix result = mGlobalMatrix;
    result[3] = VECTOR_IDENTITY_ROW_3;
    return result;
}

Matrix Entity::GetLocalOrientation() const
{
    Matrix result = mLocalMatrix;
    result[3] = VECTOR_IDENTITY_ROW_3;
    return result;
}

void Entity::SetGlobalMatrix(const Matrix& matrix)
{
    mFlags |= NFE_ENTITY_FLAG_MOVED_GLOBAL;
    mGlobalMatrix = matrix;

    HasChanged();
}

void Entity::SetGlobalPosition(const Vector& pos)
{
    mFlags |= NFE_ENTITY_FLAG_MOVED_GLOBAL;
    mGlobalMatrix.r[3] = pos;
    mGlobalMatrix.r[3].f[3] = 1.0f;

    HasChanged();
}

void Entity::SetGlobalOrientation(const Quaternion& quat)
{
    mFlags |= NFE_ENTITY_FLAG_MOVED_GLOBAL;

    Matrix rotMatrix = MatrixFromQuaternion(QuaternionNormalize(quat));
    mGlobalMatrix.r[0] = rotMatrix.r[0];
    mGlobalMatrix.r[1] = rotMatrix.r[1];
    mGlobalMatrix.r[2] = rotMatrix.r[2];

    HasChanged();
}

void Entity::SetGlobalOrientation(const Matrix& orientation)
{
    mFlags |= NFE_ENTITY_FLAG_MOVED_GLOBAL;

    // normalize directions
    Vector zAxis, yAxis, xAxis;
    zAxis = VectorNormalize3(orientation[2]);
    xAxis = VectorNormalize3(VectorCross3(orientation[1], orientation[2]));
    yAxis = VectorNormalize3(VectorCross3(zAxis, xAxis));

    mGlobalMatrix.r[0] = xAxis;
    mGlobalMatrix.r[1] = yAxis;
    mGlobalMatrix.r[2] = zAxis;

    mGlobalMatrix.r[0].f[3] = 0.0f;
    mGlobalMatrix.r[1].f[3] = 0.0f;
    mGlobalMatrix.r[2].f[3] = 0.0f;

    HasChanged();
}

void Entity::SetLocalPosition(const Vector& pos)
{
    mLocalMatrix.r[3] = pos;
    mLocalMatrix.r[3].f[3] = 1.0f;

    mFlags |= NFE_ENTITY_FLAG_MOVED_LOCAL;

    HasChanged();
}

void Entity::SetLocalOrientation(const Matrix& orientation)
{
    // normalize directions
    Vector zAxis, yAxis, xAxis;
    zAxis = VectorNormalize3(orientation[2]);
    xAxis = VectorNormalize3(VectorCross3(orientation[1], orientation[2]));
    yAxis = VectorNormalize3(VectorCross3(zAxis, xAxis));

    mLocalMatrix.r[0] = xAxis;
    mLocalMatrix.r[1] = yAxis;
    mLocalMatrix.r[2] = zAxis;

    mLocalMatrix.r[0].f[3] = 0.0f;
    mLocalMatrix.r[1].f[3] = 0.0f;
    mLocalMatrix.r[2].f[3] = 0.0f;

    mFlags |= NFE_ENTITY_FLAG_MOVED_LOCAL;

    HasChanged();
}

void Entity::HasChanged()
{
    // TODO: notify scene manager
}

} // namespace Scene
} // namespace NFE
