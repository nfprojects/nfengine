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
#include "nfCommon/InputStream.hpp"

#include "nfResources/CollisionShapeFile.hpp"


namespace NFE {
namespace Resource {

using namespace Math;


namespace {

NFE_INLINE btVector3 Float3ToBtVector3(const Float3& vec)
{
    return btVector3(vec.x, vec.y, vec.z);
}

} // namespace


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

bool CollisionShape::LoadFromFile(Common::FileInputStream& stream)
{
    CollisionShapeFile file;
    if (!file.Load(stream))
    {
        LOG_ERROR("Failed to load collision shape file");
        return false;
    }

    const auto& shapes = file.GetShapes();
    for (const auto& shape : shapes)
    {
        CompoundShapeChild bulletShape;

        switch (shape->shapeType)
        {
            case CollisionShapeType::TriangleMesh:
            {
                auto triMeshShape = dynamic_cast<CollisionShapeFile::TriangleMeshShape*>(shape.get());
                std::unique_ptr<btTriangleMesh> btShape(new btTriangleMesh());
                for (size_t i = 0; i < triMeshShape->indices.size(); i += 3)
                {
                    uint32 i0 = triMeshShape->indices[i    ];
                    uint32 i1 = triMeshShape->indices[i + 1];
                    uint32 i2 = triMeshShape->indices[i + 2];
                    const btVector3 v0(Float3ToBtVector3(triMeshShape->vertices[i0]));
                    const btVector3 v1(Float3ToBtVector3(triMeshShape->vertices[i1]));
                    const btVector3 v2(Float3ToBtVector3(triMeshShape->vertices[i2]));
                    btShape->addTriangle(v0, v1, v2);
                }

                bulletShape.shape.reset(new btBvhTriangleMeshShape(btShape.get(), true));
                bulletShape.mesh = std::move(btShape);
                break;
            }

            case CollisionShapeType::Box:
            {
                auto boxShape = dynamic_cast<CollisionShapeFile::BoxShape*>(shape.get());
                std::unique_ptr<btBoxShape> btShape(new btBoxShape(Float3ToBtVector3(Float3(boxShape->halfSize))));
                bulletShape.shape = std::move(btShape);
                break;
            }

            case CollisionShapeType::Cylinder:
            {
                auto cylinderShape = dynamic_cast<CollisionShapeFile::CylinderShape*>(shape.get());
                btVector3 halfExtents = btVector3(cylinderShape->radius, cylinderShape->height * 0.5f, cylinderShape->radius);
                std::unique_ptr<btCylinderShape> btShape(new btCylinderShape(halfExtents));
                bulletShape.shape = std::move(btShape);
                break;
            }

            default:
                LOG_WARNING("Unsupported collision shape type");
                continue;
        }

        bulletShape.matrix = MatrixFromQuaternion(shape->orientation) * MatrixTranslation3(shape->translation);
        mChildren.push_back(std::move(bulletShape));
    }

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
        // find relative path
        std::string path = g_CookedDataPath + "CollisionShapes/" + mName;

        Common::FileInputStream stream(path.c_str());
        ret = LoadFromFile(stream);
    }
    else
    {
        // run user callback
        ret = mOnLoad(this, mUserPtr);
    }

    // run custom routine
    if (ret)
    {
        if (mChildren.size() > 0)
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
