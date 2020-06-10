#include "PCH.h"
#include "SceneObject.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::ISceneObject)
{
    NFE_CLASS_MEMBER(mTransform);
    // TODO velocity
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace RT {

using namespace Math;

ISceneObject::ISceneObject()
{
    mBaseTransform = Matrix4::Identity();
    mInverseTranform = Matrix4::Identity();
}

ISceneObject::~ISceneObject() = default;

bool ISceneObject::OnPropertyChanged(const Common::StringView propertyName)
{
    if (propertyName == "mTransform")
    {
        NFE_ASSERT(mTransform.IsValid(), "");
        mBaseTransform = mTransform.ToMatrix();
        mInverseTranform = mTransform.Inverted().ToMatrix();
        return true;
    }

    return IObject::OnPropertyChanged(propertyName);
}

void ISceneObject::SetTransform(const Math::Matrix4& matrix)
{
    NFE_ASSERT(matrix.IsValid(), "");

    mTransform = Transform::FromMatrix(matrix);

    mBaseTransform = matrix;

    // TODO scaling support
    mInverseTranform = matrix.Inverted();
}

const Matrix4 ISceneObject::GetTransform(const float t) const
{
    NFE_UNUSED(t);
    NFE_ASSERT(t >= 0.0f && t <= 1.0f, "");

    // TODO motion blur

    return mBaseTransform;
}

const Matrix4 ISceneObject::GetInverseTransform(const float t) const
{
    NFE_UNUSED(t);
    NFE_ASSERT(t >= 0.0f && t <= 1.0f, "");

    // TODO motion blur

    return mInverseTranform;
}

} // namespace RT
} // namespace NFE
