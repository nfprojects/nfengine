/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  CameraComponent definitions.
 */

#include "../PCH.hpp"
#include "CameraComponent.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

NFE_REGISTER_COMPONENT(CameraComponent);

CameraComponent::CameraComponent()
{
    projMode = ProjectionMode::Perspective;
}

void CameraComponent::SetPerspective(const Perspective* desc)
{
    projMode = ProjectionMode::Perspective;
    perspective = *desc;
}

void CameraComponent::GetPerspective(Perspective* desc) const
{
    *desc = perspective;
}

void CameraComponent::SetOrtho(const Ortho* desc)
{
    projMode = ProjectionMode::Ortho;
    ortho = *desc;
}

void CameraComponent::Update(const Matrix& matrix, const Vector& velocity, const Vector& angularVelocity,
                             float dt)
{
    // calculate view matrix
    mViewMatrix = MatrixLookTo(matrix.r[3], matrix.r[2], matrix.r[1]);
    mViewMatrixInv = MatrixInverse(mViewMatrix);

    // calculate projection matrix
    if (projMode == ProjectionMode::Perspective)
        mProjMatrix = MatrixPerspective(perspective.aspectRatio, perspective.FoV,
                                        perspective.farDist, perspective.nearDist);
    else
        mProjMatrix = MatrixOrtho(ortho.left, ortho.right, ortho.bottom, ortho.top,
                                  ortho.nearDist, ortho.farDist);
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

    if (projMode == ProjectionMode::Perspective)
    {
        float y = tanf(perspective.FoV / 2.0f);
        float x = perspective.aspectRatio * y;
        mScreenScale = Vector(x, y, 0, 0);

        xAxis = x * matrix.GetRow(0);
        yAxis = y * matrix.GetRow(1);
        zAxis = matrix.GetRow(2);

        mFrustum.verticies[0] = pos + perspective.nearDist * (zAxis - xAxis - yAxis);
        mFrustum.verticies[1] = pos + perspective.nearDist * (zAxis + xAxis - yAxis);
        mFrustum.verticies[2] = pos + perspective.nearDist * (zAxis - xAxis + yAxis);
        mFrustum.verticies[3] = pos + perspective.nearDist * (zAxis + xAxis + yAxis);

        mFrustum.verticies[4] = pos + perspective.farDist * (zAxis - xAxis - yAxis);
        mFrustum.verticies[5] = pos + perspective.farDist * (zAxis + xAxis - yAxis);
        mFrustum.verticies[6] = pos + perspective.farDist * (zAxis - xAxis + yAxis);
        mFrustum.verticies[7] = pos + perspective.farDist * (zAxis + xAxis + yAxis);
    }
    else
    {
        xAxis = matrix.GetRow(0);
        yAxis = matrix.GetRow(1);
        zAxis = matrix.GetRow(2);
        mScreenScale = Vector(1, 1, 0, 0);

        mFrustum.verticies[0] = pos + ortho.nearDist * zAxis + ortho.left * xAxis  + ortho.bottom *
                                yAxis;
        mFrustum.verticies[1] = pos + ortho.nearDist * zAxis + ortho.right * xAxis + ortho.bottom *
                                yAxis;
        mFrustum.verticies[2] = pos + ortho.nearDist * zAxis + ortho.left * xAxis  + ortho.top *
                                yAxis;
        mFrustum.verticies[3] = pos + ortho.nearDist * zAxis + ortho.right * xAxis + ortho.top *
                                yAxis;

        mFrustum.verticies[4] = pos + ortho.farDist * zAxis + ortho.left * xAxis   + ortho.bottom *
                                yAxis;
        mFrustum.verticies[5] = pos + ortho.farDist * zAxis + ortho.right * xAxis  + ortho.bottom *
                                yAxis;
        mFrustum.verticies[6] = pos + ortho.farDist * zAxis + ortho.left * xAxis   + ortho.top *
                                yAxis;
        mFrustum.verticies[7] = pos + ortho.farDist * zAxis + ortho.right * xAxis  + ortho.top *
                                yAxis;
    }

    mFrustum.CalculatePlanes();
}

// Used for Cascaded Shadow Mapping to calculate shadow cascades.
// This function doesn't calculate frustum's planes
void CameraComponent::SplitFrustum(const Matrix& matrix, float zn, float zf, Frustum* frustum)
{
    Vector xAxis, yAxis, zAxis;
    Vector pos = matrix.GetRow(3);

    if (projMode == ProjectionMode::Perspective)
    {
        float y = tanf(perspective.FoV / 2.0f);
        float x = perspective.aspectRatio * y;
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

        frustum->verticies[0] = pos + zn * zAxis + ortho.left * xAxis  + ortho.bottom * yAxis;
        frustum->verticies[1] = pos + zn * zAxis + ortho.right * xAxis + ortho.bottom * yAxis;
        frustum->verticies[2] = pos + zn * zAxis + ortho.left * xAxis  + ortho.top * yAxis;
        frustum->verticies[3] = pos + zn * zAxis + ortho.right * xAxis + ortho.top * yAxis;

        frustum->verticies[4] = pos + zf * zAxis + ortho.left * xAxis  + ortho.bottom * yAxis;
        frustum->verticies[5] = pos + zf * zAxis + ortho.right * xAxis + ortho.bottom * yAxis;
        frustum->verticies[6] = pos + zf * zAxis + ortho.left * xAxis  + ortho.top * yAxis;
        frustum->verticies[7] = pos + zf * zAxis + ortho.right * xAxis + ortho.top * yAxis;
    }
}

} // namespace Scene
} // namespace NFE
