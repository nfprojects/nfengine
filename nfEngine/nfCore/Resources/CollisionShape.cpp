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
#include "nfCommon/Memory/DefaultAllocator.hpp"

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


CollisionShape::CollisionShape()
{
    mLocalInertia = Vector();
}

CollisionShape::~CollisionShape()
{
    Release();
}

void CollisionShape::Release()
{
    std::recursive_mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::unique_lock<std::recursive_mutex> lock(renderingMutex);

    mShape.reset();
    mChildren.clear();
}

bool CollisionShape::LoadFromFile(const char* pPath)
{
    //open file
    FILE* file = 0;
    if (fopen_s(&file, pPath, "rb") != 0)
    {
        //error, could not open file
        LOG_ERROR("Failed to load '%s'.", mName);
        return false;
    }

    //iterate throug all shapes
    while (!feof(file))
    {
        ShapeHeader header;
        if (fread(&header, sizeof(header), 1, file) == 0)
            continue;

        if (header.type == SHAPE_TYPE_TRI_MESH)
        {
            ShapeTraingleMesh triMeshInfo;
            fread(&triMeshInfo, sizeof(triMeshInfo), 1, file);

            float* vertices = reinterpret_cast<float*>(
                NFE_MALLOC(sizeof(float) * 3 * triMeshInfo.verticesCount, 16));
            TriMeshTriangle* triangles = reinterpret_cast<TriMeshTriangle*>(
                NFE_MALLOC(sizeof(TriMeshTriangle) * triMeshInfo.trianglesCount, 16));

            fread(vertices, sizeof(float) * 3 * triMeshInfo.verticesCount, 1, file);
            fread(triangles, sizeof(TriMeshTriangle) * triMeshInfo.trianglesCount, 1, file);

            std::unique_ptr<btTriangleMesh> mesh(new btTriangleMesh());
            btVector3 v1, v2, v3;

            for (UINT i = 0; i < triMeshInfo.trianglesCount; i++)
            {
                v1 = btVector3(vertices[3 * triangles[i].indices[0]], vertices[3 * triangles[i].indices[0] + 1],
                               vertices[3 * triangles[i].indices[0] + 2]);
                v2 = btVector3(vertices[3 * triangles[i].indices[1]], vertices[3 * triangles[i].indices[1] + 1],
                               vertices[3 * triangles[i].indices[1] + 2]);
                v3 = btVector3(vertices[3 * triangles[i].indices[2]], vertices[3 * triangles[i].indices[2] + 1],
                               vertices[3 * triangles[i].indices[2] + 2]);
                mesh->addTriangle(v1, v2, v3);
            }

            NFE_FREE(triangles);
            NFE_FREE(vertices);

            CompoundShapeChild shape;
            shape.shape.reset(new btBvhTriangleMeshShape(mesh.get(), true));
            shape.matrix = Matrix();
            shape.mesh = std::move(mesh);
            mChildren.push_back(std::move(shape));
        }
        else
        {
            LOG_ERROR("Unknown shape type.", mName);
            return false;
        }
    }

    fclose(file);
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
    shape.shape.reset(new btBoxShape(btVector3(halfSize.f[0], halfSize.f[1], halfSize.f[2])));
    shape.matrix = matrix;
    mChildren.push_back(std::move(shape));

    return true;
}

bool CollisionShape::AddCylinder(float h, float r)
{
    CompoundShapeChild shape;
    shape.shape.reset(new btCylinderShape(btVector3(r, h * 0.5f, r)));
    shape.matrix = Matrix();
    mChildren.push_back(std::move(shape));

    return true;
}

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
            btCompoundShape* compound = new btCompoundShape(true);
            if (!compound) //check allocation
            {
                LOG_ERROR("Memory allocation error ocurred during loading collision shape resource '%s'.", mName);
                return false;
            }

            for (size_t i = 0; i < mChildren.size(); i++)
                compound->addChildShape(Matrix2BulletTransform(mChildren[i].matrix), mChildren[i].shape.get());

            mShape.reset(compound);
        }
        else if (mChildren.size() == 1) //single shape
        {
            mShape = std::move(mChildren[0].shape);
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
