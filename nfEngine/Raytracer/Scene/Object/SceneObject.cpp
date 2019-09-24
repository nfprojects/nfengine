#include "PCH.h"
#include "SceneObject.h"

namespace NFE {
namespace RT {

using namespace Math;

ISceneObject::ISceneObject()
{
    mTransform = Matrix4::Identity();
    mInverseTranform = Matrix4::Identity();
}

ISceneObject::~ISceneObject() = default;

void ISceneObject::SetTransform(const Math::Matrix4& matrix)
{
    NFE_ASSERT(matrix.IsValid());

    mTransform = matrix;

    // TODO scaling support
    mInverseTranform = matrix.Inverted();
}

const Matrix4 ISceneObject::GetTransform(const float t) const
{
    NFE_UNUSED(t);
    NFE_ASSERT(t >= 0.0f && t <= 1.0f);

    // TODO motion blur

    return mTransform;
}

const Matrix4 ISceneObject::GetInverseTransform(const float t) const
{
    NFE_UNUSED(t);
    NFE_ASSERT(t >= 0.0f && t <= 1.0f);

    // TODO motion blur

    return mInverseTranform;
}

} // namespace RT
} // namespace NFE
