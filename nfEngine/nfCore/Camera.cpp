/**
    NFEngine project

    \file   Camera.cpp
    \brief  Camera component declaration.
*/

#include "stdafx.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "..\nfCommon\InputStream.hpp"
#include "..\nfCommon\OutputStream.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

Camera::Camera(Entity* pParent) : Component(pParent)
{
    mType = ComponentType::Camera;
    projMode = ProjectionMode::Perspective;

    mParentOffset = MatrixIdentity();
}

void Camera::SetPerspective(const Perspective* pDesc)
{
    projMode = ProjectionMode::Perspective;
    perspective = *pDesc;
}

void Camera::GetPerspective(Perspective* pDesc) const
{
    *pDesc = perspective;
}

void Camera::SetOrtho(const Ortho* pDesc)
{
    projMode = ProjectionMode::Ortho;
    ortho = *pDesc;
}

void Camera::OnUpdate(float dt)
{
    Matrix matrix = mParentOffset * mOwner->mMatrix;

    //calculate view matrix
    mViewMatrix = MatrixLookTo(matrix.r[3], matrix.r[2], matrix.r[1]);
    mViewMatrixInv = MatrixInverse(mViewMatrix);

    //calculat projection matrix
    if (projMode == ProjectionMode::Perspective)
        mProjMatrix = MatrixPerspective(perspective.aspectRatio, perspective.FoV,
                                        perspective.farDist, perspective.nearDist);
    else
        mProjMatrix = MatrixOrtho(ortho.left, ortho.right, ortho.bottom, ortho.top,
                                  ortho.nearDist, ortho.farDist);
    mProjMatrixInv = MatrixInverse(mProjMatrix);



    //calculate secondary view matrix for motion blur
    Matrix rotMatrix = MatrixRotationNormal(mOwner->mAngularVelocity, 0.01f);
    Matrix secondaryCameraMatrix;
    secondaryCameraMatrix.r[0] = VectorTransform3(matrix.r[0], rotMatrix);
    secondaryCameraMatrix.r[1] = VectorTransform3(matrix.r[1], rotMatrix);
    secondaryCameraMatrix.r[2] = VectorTransform3(matrix.r[2], rotMatrix);
    secondaryCameraMatrix.r[3] = (Vector)matrix.r[3] + mOwner->mVelocity * 0.01f;
    Matrix secondaryViewMatrix = MatrixLookTo(secondaryCameraMatrix.r[3], secondaryCameraMatrix.r[2],
                                 secondaryCameraMatrix.r[1]);
    mSecondaryProjViewMatrix = secondaryViewMatrix * mProjMatrix;

    /*
    //calculate secondary view matrix for motion blur
    Matrix prevViewMatrix = MatrixLookTo(mPrevMatrix.r[3], mPrevMatrix.r[2], mPrevMatrix.r[1]);
    mSecondaryProjViewMatrix = prevViewMatrix * mProjMatrix;
    */

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


//Used for Cascaded Shadow Mapping to calculate shadow cascades.
//This function doesn't calculate frustum's planes
void Camera::SplitFrustum(float zn, float zf, Frustum* pFrustum)
{
    Matrix matrix = mOwner->mMatrix * mParentOffset;

    Vector xAxis, yAxis, zAxis;
    Vector pos = matrix.GetRow(3);

    if (projMode == ProjectionMode::Perspective)
    {
        float y = tanf(perspective.FoV / 2.0f);
        float x = perspective.aspectRatio * y;
        xAxis = x * matrix.GetRow(0);
        yAxis = y * matrix.GetRow(1);
        zAxis = matrix.GetRow(2);

        pFrustum->verticies[0] = pos + zn * (zAxis - xAxis - yAxis);
        pFrustum->verticies[1] = pos + zn * (zAxis + xAxis - yAxis);
        pFrustum->verticies[2] = pos + zn * (zAxis - xAxis + yAxis);
        pFrustum->verticies[3] = pos + zn * (zAxis + xAxis + yAxis);

        pFrustum->verticies[4] = pos + zf * (zAxis - xAxis - yAxis);
        pFrustum->verticies[5] = pos + zf * (zAxis + xAxis - yAxis);
        pFrustum->verticies[6] = pos + zf * (zAxis - xAxis + yAxis);
        pFrustum->verticies[7] = pos + zf * (zAxis + xAxis + yAxis);
    }
    else
    {
        xAxis = matrix.GetRow(0);
        yAxis = matrix.GetRow(1);
        zAxis = matrix.GetRow(2);

        pFrustum->verticies[0] = pos + zn * zAxis + ortho.left * xAxis  + ortho.bottom * yAxis;
        pFrustum->verticies[1] = pos + zn * zAxis + ortho.right * xAxis + ortho.bottom * yAxis;
        pFrustum->verticies[2] = pos + zn * zAxis + ortho.left * xAxis  + ortho.top * yAxis;
        pFrustum->verticies[3] = pos + zn * zAxis + ortho.right * xAxis + ortho.top * yAxis;

        pFrustum->verticies[4] = pos + zf * zAxis + ortho.left * xAxis  + ortho.bottom * yAxis;
        pFrustum->verticies[5] = pos + zf * zAxis + ortho.right * xAxis + ortho.bottom * yAxis;
        pFrustum->verticies[6] = pos + zf * zAxis + ortho.left * xAxis  + ortho.top * yAxis;
        pFrustum->verticies[7] = pos + zf * zAxis + ortho.right * xAxis + ortho.top * yAxis;
    }
}

// TODO
Result Camera::Deserialize(Common::InputStream* pStream)
{
    CameraDesc desc;
    if (pStream->Read(sizeof(desc), &desc) != sizeof(desc))
        return Result::Error;

    return Result::OK;
}

// TODO
Result Camera::Serialize(Common::OutputStream* pStream) const
{
    return Result::OK;
}

} // namespace Scene
} // namespace NFE
