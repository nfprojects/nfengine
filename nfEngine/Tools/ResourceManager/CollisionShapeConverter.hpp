/**
* @file
* @author Witek902 (witek902@gmail.com)
* @brief  Collision shape converter declarations
*/

#pragma once

#include "nfCommon/Math/Math.hpp"
#include "nfCommon/Math/Quaternion.hpp"

#include <vector>


namespace NFE {

// TODO
// * support other shapes (convex hull, sphere, box, cylinder, etc.)
// * support for physics materials
// * saving cooked Bullet objects for faster loading in the engine?

#define NFE_SHAPE_TYPE_TRI_MESH     1
#define NFE_SHAPE_TYPE_CONVEX_HULL  2
#define NFE_SHAPE_TYPE_BOX          3
#define NFE_SHAPE_TYPE_SPHERE       4

enum class CollisionShapeType : uint32
{
    Invalid         = 0xFFFFFFFF,
    TriangleMesh    = 0,
    ConvexHull,

    // TODO more shapes
};

/**
 * Helper class allowing for mesh file conversion.
 */
class CollisionShapeFile
{
public:
    struct ShapeHeader
    {
        CollisionShapeType shapeType;
        Math::Float3 translation;
        Math::Quaternion orientation;

        NFE_INLINE ShapeHeader()
        {
            shapeType = CollisionShapeType::Invalid;
            orientation = Math::QuaternionIdentity();
        }
    };

    struct IShape
    {
        CollisionShapeType type;
        virtual ~IShape() { }
    };

    struct TriangleMesh : public IShape
    {
        std::vector<Math::Float3> vertices;
        std::vector<unsigned int> indices;
    };

    /**
     * Load mesh form OBJ file.
     */
    bool LoadFromObj(const std::string& path);

    /**
     * Save mesh to binary (cooked) format.
     */
    bool Save(const std::string& path) const;

private:
    std::vector<Vertex> mVertices;
    std::vector<unsigned int> mIndices;
    std::vector<SubMesh> mSubMeshes;
    std::vector<Material> mMaterials;
};

} // namespace NFE