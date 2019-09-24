/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  CameraComponent definitions.
 */

#include "PCH.hpp"
#include "ComponentCamera.hpp"
#include "../Entity.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::CameraComponent)
    NFE_CLASS_PARENT(NFE::Scene::IComponent)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

using namespace Math;
using namespace Resource;


CameraComponent::CameraComponent()
    : mProjMode(ProjectionMode::Perspective)
{
}

Box CameraComponent::GetBoundingBox() const
{
    // TODO
    return Box();
}

void CameraComponent::SetPerspective(const PerspectiveProjectionDesc* desc)
{
    mProjMode = ProjectionMode::Perspective;
    mPerspective = *desc;
}

void CameraComponent::GetPerspective(PerspectiveProjectionDesc* desc) const
{
    *desc = mPerspective;
}

void CameraComponent::SetOrtho(const OrthoProjectionDesc* desc)
{
    mProjMode = ProjectionMode::Ortho;
    mOrtho = *desc;
}

void CameraComponent::GetOrtho(OrthoProjectionDesc* desc) const
{
    *desc = mOrtho;
}

void CameraComponent::OnUpdate()
{
    NFE_ASSERT(GetEntity(), "Updating camera component not attached to an entity");
    const Matrix4& matrix = GetEntity()->GetGlobalTransform().ToMatrix();

    // TODO get velocity from body component
    Update(matrix, Vector4::Zero(), Vector4::Zero());
}

void CameraComponent::Update(const Matrix4& matrix, const Vector4& velocity, const Vector4& angularVelocity)
{
    // calculate view matrix
    mViewMatrix = Matrix4::MakeLookTo(matrix.rows[3], matrix.rows[2], matrix.rows[1]);
    mViewMatrixInv = mViewMatrix.Inverted();

    // calculate projection matrix
    if (mProjMode == ProjectionMode::Perspective)
    {
        mProjMatrix = Matrix4::MakePerspective(mPerspective.aspectRatio, mPerspective.FoV,
                                              mPerspective.farDist, mPerspective.nearDist);
    }
    else
    {
        mProjMatrix = Matrix4::MakeOrtho(mOrtho.left, mOrtho.right, mOrtho.bottom, mOrtho.top,
                                        mOrtho.nearDist, mOrtho.farDist);
    }
    mProjMatrixInv = mProjMatrix.Inverted();


    //calculate secondary view matrix for motion blur
    Matrix4 rotMatrix = Matrix4::MakeRotationNormal(angularVelocity, 0.01f);
    Matrix4 secondaryCameraMatrix;
    secondaryCameraMatrix[0] = rotMatrix.TransformPoint(matrix.rows[0]);
    secondaryCameraMatrix[1] = rotMatrix.TransformPoint(matrix.rows[1]);
    secondaryCameraMatrix[2] = rotMatrix.TransformPoint(matrix.rows[2]);
    secondaryCameraMatrix[3] = (Vector4)matrix.rows[3] + velocity * 0.01f;
    Matrix4 secondaryViewMatrix = Matrix4::MakeLookTo(secondaryCameraMatrix.rows[3],
                                                      secondaryCameraMatrix.rows[2],
                                                      secondaryCameraMatrix.rows[1]);
    mSecondaryProjViewMatrix = secondaryViewMatrix * mProjMatrix;

    Vector4 xAxis, yAxis, zAxis;
    Vector4 pos = matrix.GetRow(3);

    if (mProjMode == ProjectionMode::Perspective)
    {
        float y = tanf(mPerspective.FoV / 2.0f);
        float x = mPerspective.aspectRatio * y;
        mScreenScale = Vector4(x, y);

        xAxis = x * matrix.GetRow(0);
        yAxis = y * matrix.GetRow(1);
        zAxis = matrix.GetRow(2);

        mFrustum.verticies[0] = pos + mPerspective.nearDist * (zAxis - xAxis - yAxis);
        mFrustum.verticies[1] = pos + mPerspective.nearDist * (zAxis + xAxis - yAxis);
        mFrustum.verticies[2] = pos + mPerspective.nearDist * (zAxis - xAxis + yAxis);
        mFrustum.verticies[3] = pos + mPerspective.nearDist * (zAxis + xAxis + yAxis);

        mFrustum.verticies[4] = pos + mPerspective.farDist * (zAxis - xAxis - yAxis);
        mFrustum.verticies[5] = pos + mPerspective.farDist * (zAxis + xAxis - yAxis);
        mFrustum.verticies[6] = pos + mPerspective.farDist * (zAxis - xAxis + yAxis);
        mFrustum.verticies[7] = pos + mPerspective.farDist * (zAxis + xAxis + yAxis);
    }
    else
    {
        xAxis = matrix.GetRow(0);
        yAxis = matrix.GetRow(1);
        zAxis = matrix.GetRow(2);
        mScreenScale = Vector4(1, 1, 0, 0);

        mFrustum.verticies[0] = pos + mOrtho.nearDist * zAxis + mOrtho.left * xAxis + mOrtho.bottom * yAxis;
        mFrustum.verticies[1] = pos + mOrtho.nearDist * zAxis + mOrtho.right * xAxis + mOrtho.bottom * yAxis;
        mFrustum.verticies[2] = pos + mOrtho.nearDist * zAxis + mOrtho.left * xAxis + mOrtho.top * yAxis;
        mFrustum.verticies[3] = pos + mOrtho.nearDist * zAxis + mOrtho.right * xAxis + mOrtho.top * yAxis;

        mFrustum.verticies[4] = pos + mOrtho.farDist * zAxis + mOrtho.left * xAxis + mOrtho.bottom * yAxis;
        mFrustum.verticies[5] = pos + mOrtho.farDist * zAxis + mOrtho.right * xAxis + mOrtho.bottom * yAxis;
        mFrustum.verticies[6] = pos + mOrtho.farDist * zAxis + mOrtho.left * xAxis + mOrtho.top * yAxis;
        mFrustum.verticies[7] = pos + mOrtho.farDist * zAxis + mOrtho.right * xAxis + mOrtho.top * yAxis;
    }

    mFrustum.CalculatePlanes();
}

void CameraComponent::SplitFrustum(const Matrix4& matrix, float zn, float zf, Frustum* frustum) const
{
    Vector4 xAxis, yAxis, zAxis;
    Vector4 pos = matrix.GetRow(3);

    if (mProjMode == ProjectionMode::Perspective)
    {
        float y = tanf(mPerspective.FoV / 2.0f);
        float x = mPerspective.aspectRatio * y;
        xAxis = x * matrix.GetRow(0);
        yAxis = y * matrix.GetRow(1);
        zAxis = matrix.GetRow(2);

        frustum->verticies[0] = pos + zn * (zAxis - xAxis - yAxis);
        frustum->verticies[1] = pos + zn * (zAxis + xAxis - yAxis);
        frustum->verticies[2] = pos + zn * (zAxis - xAxis + yAxis);
        frustum->verticies[3] = pos + zn * (zAxis + xAxis + yAxis);

        frustum->verticies[4] = pos + zf * (zAxis - xAxis - yAxis);
        frustum->verticies[5] = pos + zf * (zAxis + xAxis - yAxis);
        frustum->verticies[6] = pos + zf * (zAxis - xAxis + yAxis);
        frustum->verticies[7] = pos + zf * (zAxis + xAxis + yAxis);
    }
    else
    {
        xAxis = matrix.GetRow(0);
        yAxis = matrix.GetRow(1);
        zAxis = matrix.GetRow(2);

        frustum->verticies[0] = pos + zn * zAxis + mOrtho.left * xAxis  + mOrtho.bottom * yAxis;
        frustum->verticies[1] = pos + zn * zAxis + mOrtho.right * xAxis + mOrtho.bottom * yAxis;
        frustum->verticies[2] = pos + zn * zAxis + mOrtho.left * xAxis  + mOrtho.top * yAxis;
        frustum->verticies[3] = pos + zn * zAxis + mOrtho.right * xAxis + mOrtho.top * yAxis;

        frustum->verticies[4] = pos + zf * zAxis + mOrtho.left * xAxis  + mOrtho.bottom * yAxis;
        frustum->verticies[5] = pos + zf * zAxis + mOrtho.right * xAxis + mOrtho.bottom * yAxis;
        frustum->verticies[6] = pos + zf * zAxis + mOrtho.left * xAxis  + mOrtho.top * yAxis;
        frustum->verticies[7] = pos + zf * zAxis + mOrtho.right * xAxis + mOrtho.top * yAxis;
    }
}

} // namespace Scene
} // namespace NFE
