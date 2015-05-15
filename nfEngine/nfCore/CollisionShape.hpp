/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Collision shape resource declarations.
 */

#pragma once

#include "Core.hpp"
#include "Resource.hpp"

//predeclarations
class btCollisionShape;
class btCompoundShape;

namespace NFE {
namespace Resource {

NFE_ALIGN(16)
struct CompoundShapeChild
{
    btCollisionShape* pShape;
    Math::Matrix matrix;
};

class CORE_API CollisionShape : public ResourceBase
{
    friend class Scene::BodyComponent;

private:
    std::vector<CompoundShapeChild> mChildren;
    btCollisionShape* mShape;
    Math::Vector mLocalInertia;

    void Release();

public:
    CollisionShape();
    virtual ~CollisionShape();

    static CollisionShape* Allocate();
    static void Free(CollisionShape* ptr);

    bool OnLoad();
    void OnUnload();

    // TODO: this function should operate on Common::InputStream
    Result LoadFromFile(const char* pPath);

    // Add box shape
    Result AddBox(const Math::Vector& halfSize, const Math::Matrix& matrix);

    // Add cylinder shape
    Result AddCylinder(float h, float r);
};

} // namespace Resource
} // namespace NFE
