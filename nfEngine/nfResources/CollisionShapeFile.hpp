/**
* @file
* @author Witek902 (witek902@gmail.com)
* @brief  Collision shape converter declarations
*/

#pragma once

#include "nfCommon/Math/Math.hpp"
#include "nfCommon/Math/Quaternion.hpp"
#include "nfCommon/InputStream.hpp"
#include "nfCommon/OutputStream.hpp"
#include "nfCommon/Aligned.hpp"

#include <vector>
#include <memory>


#define NFE_COLLISION_SHAPE_FILE_MAGIC 'scfn'   // "nfcs"

namespace NFE {
namespace Resource {

// TODO
// * support more shapes
// * support for physics materials
// * saving cooked Bullet objects for faster loading in the engine?

enum class CollisionShapeType : uint16
{
    Invalid = 0xFFFF,
    TriangleMesh = 0,
    Box,
    Cylinder,

    // TODO more shapes
};

/**
 * Helper class allowing for mesh file conversion.
 */
class CollisionShapeFile
{
public:
    struct FileHeader
    {
        uint32 magic;   // must be equal to NFE_COLLISION_SHAPE_FILE_MAGIC
        uint32 numShapes;
        uint32 numMaterials;
    };

    struct NFE_ALIGN16 Shape : public Common::Aligned<16>
    {
        CollisionShapeType shapeType;
        uint16 defaultMaterialID;
        Math::Vector translation;
        Math::Quaternion orientation;

        // TODO density / center of mass information

        NFE_INLINE Shape()
            : shapeType(CollisionShapeType::Invalid)
            , defaultMaterialID(0)
        {
            orientation = Math::QuaternionIdentity();
        }

        virtual ~Shape() { }

        // TODO this should be handle via InputBuffer and OutputBuffer (or sth else)
        virtual bool OnLoad(Common::InputStream& stream) = 0;
        virtual bool OnSave(Common::OutputStream& stream) const = 0;

        static std::unique_ptr<Shape> Load(Common::InputStream& stream);
        bool Save(Common::OutputStream& stream) const;
    };


    class TriangleMeshShape : public Shape
    {
        struct Header
        {
            uint32 verticesNum;
            uint32 trianglesNum;
            bool overrideMaterials;
        };

    public:
        std::vector<Math::Float3> vertices;
        std::vector<uint32> indices;

        bool overrideMaterials;             // if set to "true" materialIDs will override "defaultMaterialID"
        std::vector<uint16> materialIDs;    // per triangle material IDs

        NFE_INLINE TriangleMeshShape()
            : overrideMaterials(false)
        {
            shapeType = CollisionShapeType::TriangleMesh;
        }

        bool OnLoad(Common::InputStream& stream) override;
        bool OnSave(Common::OutputStream& stream) const override;
    };


    struct BoxShape : public Shape
    {
        Math::Vector halfSize;

        NFE_INLINE BoxShape()
        {
            shapeType = CollisionShapeType::Box;
        }

        bool OnLoad(Common::InputStream& stream) override;
        bool OnSave(Common::OutputStream& stream) const override;
    };


    struct CylinderShape : public Shape
    {
        float height;
        float radius;

        NFE_INLINE CylinderShape()
        {
            shapeType = CollisionShapeType::Cylinder;
        }

        bool OnLoad(Common::InputStream& stream) override;
        bool OnSave(Common::OutputStream& stream) const override;
    };

    /**
     * Load collision shape from binary format.
     */
    bool Load(Common::InputStream& stream);

    /**
     * Save collision shape to binary (cooked) format.
     */
    bool Save(Common::OutputStream& stream) const;


    NFE_INLINE const std::vector<std::unique_ptr<Shape>>& GetShapes() const
    {
        return mShapes;
    }

protected:
    std::vector<std::unique_ptr<Shape>> mShapes;
};

} // namespace Resource
} // namespace NFE