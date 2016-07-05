/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Collision shape resource definition.
 */

#include "PCH.hpp"
#include "CollisionShape.hpp"
#include "Globals.hpp"
#include "Engine.hpp"

#include "nfCommon/Logger.hpp"
#include "nfCommon/Timer.hpp"

#include "btBulletCollisionCommon.h"


namespace NFE {
namespace Resource {

using namespace Math;

#define SHAPE_TYPE_TRI_MESH (1)

struct ShapeHeader
{
    UINT type;
    float translation[3];
    float orientation[3][3];
};

struct ShapeTraingleMesh
{
    UINT verticesCount;
    UINT trianglesCount;
};

struct TriMeshTriangle
{
    UINT indices[3];
    UINT materialID;    // unused
};



CollisionShape* CollisionShape::Allocate()
{
    return new CollisionShape;
}

void CollisionShape::Free(CollisionShape* ptr)
{
    delete ptr;
}

CollisionShape::CollisionShape()
{
    mShape = 0;
    mLocalInertia = Vector();
}

CollisionShape::~CollisionShape()
{
    Release();
}

void CollisionShape::Release()
{
    std::mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::unique_lock<std::mutex> lock(renderingMutex);

    if (mChildren.size() > 1)
    {
        if (mShape)
        {
            delete mShape;
            mShape = 0;
        }

        for (size_t i = 0; i < mChildren.size(); i++)
            delete mChildren[i].pShape;
    }
    else if (mChildren.size() == 1)
    {
        if (mShape)
        {
            delete mShape;
            mShape = 0;
        }
    }

    mChildren.clear();
}

bool CollisionShape::LoadFromFile(const char* pPath)
{
    //open file
    FILE* pFile = 0;
    if (fopen_s(&pFile, pPath, "rb") != 0)
    {
        //error, could not open file
        LOG_ERROR("Failed to load '%s'.", mName);
        return false;
    }

    //iterate throug all shapes
    while (!feof(pFile))
    {
        ShapeHeader header;
        if (fread(&header, sizeof(header), 1, pFile) == 0)
            continue;

        if (header.type == SHAPE_TYPE_TRI_MESH)
        {
            ShapeTraingleMesh triMeshInfo;
            fread(&triMeshInfo, sizeof(triMeshInfo), 1, pFile);



            float* pVertices = (float*)_aligned_malloc(sizeof(float) * 3 * triMeshInfo.verticesCount, 16);
            TriMeshTriangle* pTriangles = (TriMeshTriangle*)_aligned_malloc(sizeof(
                                              TriMeshTriangle) * triMeshInfo.trianglesCount, 16);
            fread(pVertices, sizeof(float) * 3 * triMeshInfo.verticesCount, 1, pFile);
            fread(pTriangles, sizeof(TriMeshTriangle) * triMeshInfo.trianglesCount, 1, pFile);

            btTriangleMesh* pMesh = new btTriangleMesh();
            btVector3 v1, v2, v3;

            for (UINT i = 0; i < triMeshInfo.trianglesCount; i++)
            {
                v1 = btVector3(pVertices[3 * pTriangles[i].indices[0]], pVertices[3 * pTriangles[i].indices[0] + 1],
                               pVertices[3 * pTriangles[i].indices[0] + 2]);
                v2 = btVector3(pVertices[3 * pTriangles[i].indices[1]], pVertices[3 * pTriangles[i].indices[1] + 1],
                               pVertices[3 * pTriangles[i].indices[1] + 2]);
                v3 = btVector3(pVertices[3 * pTriangles[i].indices[2]], pVertices[3 * pTriangles[i].indices[2] + 1],
                               pVertices[3 * pTriangles[i].indices[2] + 2]);
                pMesh->addTriangle(v1, v2, v3);
            }


            _aligned_free(pTriangles);
            _aligned_free(pVertices);


            CompoundShapeChild shape =
            {
                new btBvhTriangleMeshShape(pMesh, true),
                Matrix()
            };
            mChildren.push_back(shape);
        }
        else
        {
            LOG_ERROR("Unknown shape type.", mName);
            return false;
        }
    }

    fclose(pFile);
    return true;
}

btTransform Matrix2BulletTransform(const Matrix& matrix)
{
    btTransform transform;
    transform.setOrigin(btVector3(matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]));
    transform.setBasis(btMatrix3x3(matrix.m[0][0], matrix.m[0][1], matrix.m[0][2],
                                   matrix.m[1][0], matrix.m[1][1], matrix.m[1][2],
                                   matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]));

    return transform;
}

bool CollisionShape::AddBox(const Vector& halfSize, const Matrix& matrix)
{
    CompoundShapeChild shape;
    shape.pShape = new btBoxShape(btVector3(halfSize.f[0], halfSize.f[1], halfSize.f[2])),
    shape.matrix = matrix;
    mChildren.push_back(shape);

    return true;
}

bool CollisionShape::AddCylinder(float h, float r)
{
    CompoundShapeChild shape;
    shape.pShape = new btCylinderShape(btVector3(r, h * 0.5f, r));
    shape.matrix = Matrix();
    mChildren.push_back(shape);

    return true;
}

/*
void CollisionShape::CreateTriMesh()
{
    btTriangleMesh* pMesh = new btTriangleMesh();
    pMesh->Add

    mShape = new btBvhTriangleMeshShape(pMesh, true);
}
*/

bool CollisionShape::OnLoad()
{
    Release();

    LOG_INFO("Loading collision shape '%s'...", mName);
    Common::Timer timer;
    timer.Start();

    bool ret;

    if (mOnLoad == NULL)
    {
        //find relative path
        std::string path = g_DataPath + "CollisionShapes/" + mName;
        ret = LoadFromFile(path.c_str());
    }
    else
    {
        // run user callback
        ret = mOnLoad(this, mUserPtr);
    }

    // run custom routine
    if (ret)
    {
        if (mChildren.size() > 1) // compund shape
        {
            btCompoundShape* pCompound = new btCompoundShape(true);
            if (pCompound == NULL) //check allocation
            {
                LOG_ERROR("Memory allocation error ocurred during loading collision shape resource '%s'.", mName);
                return false;
            }

            for (size_t i = 0; i < mChildren.size(); i++)
                pCompound->addChildShape(Matrix2BulletTransform(mChildren[i].matrix), mChildren[i].pShape);

            mShape = pCompound;
        }
        else if (mChildren.size() == 1) //single shape
        {
            mShape = mChildren[0].pShape;
        }
        else
        {
            LOG_WARNING("Collision shape '%s' is empty.", mName);
            return false;
        }

        if (mShape->isNonMoving())
        {
            mLocalInertia = Vector();
        }
        else
        {
            //calculate whole shape properties
            btVector3 inertia = btVector3(0.0f, 0.0f, 0.0f);
            mShape->calculateLocalInertia(1.0f, inertia);
            mLocalInertia = Vector(inertia.m_floats);
        }

        LOG_SUCCESS("Collision shape '%s' successfully loaded in %.3f sec.", mName, timer.Stop());
        return true;
    }

    return false;
}

void CollisionShape::OnUnload()
{
    LOG_INFO("Unloading collision shape '%s'...", mName);

    if (mOnUnload)
    {
        mOnUnload(this, mUserPtr);
    }

    Release();
}

} // namespace Resource
} // namespace NFE
