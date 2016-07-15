/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Collision shape resource declarations.
 */

#pragma once

#include "../Core.hpp"
#include "Resource.hpp"

#include "nfCommon/Math/Matrix.hpp"


//predeclarations
class btCollisionShape;
class btCompoundShape;
class btTriangleMesh;

namespace NFE {
namespace Resource {

NFE_ALIGN16
struct CompoundShapeChild
{
    std::unique_ptr<btCollisionShape> shape;
    std::unique_ptr<btTriangleMesh> mesh;
    Math::Matrix matrix;
};

class CORE_API CollisionShape : public ResourceBase
{
    friend class Scene::BodyComponent;
    friend class Scene::PhysicsSystem;

private:
    std::vector<CompoundShapeChild> mChildren;
    std::unique_ptr<btCollisionShape> mShape;
    Math::Vector mLocalInertia;

    void Release();

public:
    CollisionShape();
    virtual ~CollisionShape();

    bool OnLoad();
    void OnUnload();

    // TODO: this function should operate on Common::InputStream
    bool LoadFromFile(const char* pPath);

    // Add box shape
    bool AddBox(const Math::Vector& halfSize, const Math::Matrix& matrix);

    // Add cylinder shape
    bool AddCylinder(float h, float r);
};

} // namespace Resource
} // namespace NFE
