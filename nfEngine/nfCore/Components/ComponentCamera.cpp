/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  CameraComponent definitions.
 */

#include "PCH.hpp"
#include "ComponentCamera.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

NFE_REGISTER_COMPONENT(CameraComponent);

CameraComponent::CameraComponent()
    : mProjMode(ProjectionMode::Perspective)
{
}

void CameraComponent::SetPerspective(const Perspective* desc)
{
    mProjMode = ProjectionMode::Perspective;
    mPerspective = *desc;
}

void CameraComponent::GetPerspective(Perspective* desc) const
{
    *desc = mPerspective;
}

void CameraComponent::SetOrtho(const Ortho* desc)
{
    mProjMode = ProjectionMode::Ortho;
    mOrtho = *desc;
}

void CameraComponent::GetOrtho(Ortho* desc) const
{
    *desc = mOrtho;
}

void CameraComponent::Update(const Matrix& matrix, const Vector& velocity,
                             const Vector& angularVelocity, float dt)
{
    // calculate view matrix
    mViewMatrix = MatrixLookTo(matrix.r[3], matrix.r[2], matrix.r[1]);
    mViewMatrixInv = MatrixInverse(mViewMatrix);

    // calculate projection matrix
    if (mProjMode == ProjectionMode::Perspective)
        mProjMatrix = MatrixPerspective(mPerspective.aspectRatio, mPerspective.FoV,
                                        mPerspective.farDist, mPerspective.nearDist);
    else
        mProjMatrix = MatrixOrtho(mOrtho.left, mOrtho.right, mOrtho.bottom, mOrtho.top,
                                  mOrtho.nearDist, mOrtho.farDist);
    mProjMatrixInv = MatrixInverse(mProjMatrix);


    //calculate secondary view matrix for motion blur
    Matrix rotMatrix = MatrixRotationNormal(angularVelocity, 0.01f);
    Matrix secondaryCameraMatrix;
    secondaryCameraMatrix.r[0] = LinearCombination3(matrix.r[0], rotMatrix);
    secondaryCameraMatrix.r[1] = LinearCombination3(matrix.r[1], rotMatrix);
    secondaryCameraMatrix.r[2] = LinearCombination3(matrix.r[2], rotMatrix);
    secondaryCameraMatrix.r[3] = (Vector)matrix.r[3] + velocity * 0.01f;
    Matrix secondaryViewMatrix = MatrixLookTo(secondaryCameraMatrix.r[3], secondaryCameraMatrix.r[2],
                                 secondaryCameraMatrix.r[1]);
    mSecondaryProjViewMatrix = secondaryViewMatrix * mProjMatrix;

    Vector xAxis, yAxis, zAxis;
    Vector pos = matrix.GetRow(3);

    if (mProjMode == ProjectionMode::Perspective)
    {
        float y = tanf(mPerspective.FoV / 2.0f);
        float x = mPerspective.aspectRatio * y;
        mScreenScale = Vector(x, y);

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
        mScreenScale = Vector(1, 1, 0, 0);

        mFrustum.verticies[0] = pos + mOrtho.nearDist * zAxis + mOrtho.left * xAxis
                                    + mOrtho.bottom * yAxis;
        mFrustum.verticies[1] = pos + mOrtho.nearDist * zAxis + mOrtho.right * xAxis
                                    + mOrtho.bottom * yAxis;
        mFrustum.verticies[2] = pos + mOrtho.nearDist * zAxis + mOrtho.left * xAxis
                                    + mOrtho.top * yAxis;
        mFrustum.verticies[3] = pos + mOrtho.nearDist * zAxis + mOrtho.right * xAxis
                                    + mOrtho.top * yAxis;

        mFrustum.verticies[4] = pos + mOrtho.farDist * zAxis + mOrtho.left * xAxis
                                    + mOrtho.bottom * yAxis;
        mFrustum.verticies[5] = pos + mOrtho.farDist * zAxis + mOrtho.right * xAxis
                                    + mOrtho.bottom * yAxis;
        mFrustum.verticies[6] = pos + mOrtho.farDist * zAxis + mOrtho.left * xAxis
                                    + mOrtho.top * yAxis;
        mFrustum.verticies[7] = pos + mOrtho.farDist * zAxis + mOrtho.right * xAxis
                                    + mOrtho.top * yAxis;
    }

    mFrustum.CalculatePlanes();
}

// Used for Cascaded Shadow Mapping to calculate shadow cascades.
// This function doesn't calculate frustum's planes
void CameraComponent::SplitFrustum(const Matrix& matrix, float zn, float zf,
                                   Frustum* frustum) const
{
    Vector xAxis, yAxis, zAxis;
    Vector pos = matrix.GetRow(3);

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
